#include "../../../../include/clients/unix_socket/abstract/abstract_unix_socket_client.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <exception>
#include <mutex>
#include <thread>

#include "../../../../include/utils/constants.hpp"
#include "../../../../include/utils/string_utils.hpp"
#include "../../../../include/utils/time_utils.hpp"

using json = nlohmann::json;

AbstractUnixSocketClient::AbstractUnixSocketClient(const std::string& path, const std::string& name) : Loggable(name), path(path), name(name) {
	running.store(true);
	connected.store(false);

	connectionThread = std::thread([this] {
		connectionLoop();
	});
}

AbstractUnixSocketClient::~AbstractUnixSocketClient() {
	try {
		connected = false;
		running	  = false;

		if (sock != -1) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			sock = -1;
		}

		queue_cv.notify_all();

		if (connectionThread.joinable()) {
			connectionThread.join();
		}
		if (writeThread.joinable()) {
			writeThread.join();
		}
		if (readThread.joinable()) {
			readThread.join();
		}
	} catch (std::exception& e) {
		logger.error(e.what());
	}
}

void AbstractUnixSocketClient::connectionLoop() {
	while (running) {
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		sockaddr_un addr{};
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, Constants::SOCKET_FILE.c_str(), sizeof(addr.sun_path) - 1);

		if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
			continue;
		}

		connected.store(true);

		writeThread = std::thread([this] {
			writeLoop();
		});
		readThread	= std::thread([this] {
			 readLoop();
		 });

		while (connected) {
			for (int i = 0; i < 20 && connected && running; i++) {
				TimeUtils::sleep(250);
			}
			if (!running) {
				try {
					invoke("ping", {}, 5);
				} catch (std::exception& e) {
					logger.debug("Lost connection");
				}
			}
		}
	}
}

void AbstractUnixSocketClient::writeLoop() {
	while (true) {
		std::unique_lock<std::mutex> lock(mutex);
		queue_cv.wait(lock, [this] {
			return !_message_queue.empty() || !running;
		});

		if (!running) {
			break;
		}

		if (!connected) {
			continue;
		}

		if (_message_queue.empty()) {
			continue;
		}

		std::string msg = _message_queue.front();
		_message_queue.pop();
		lock.unlock();

		uint32_t msgLen = htonl(msg.size());
		if (write(sock, &msgLen, sizeof(msgLen)) <= 0 || write(sock, msg.c_str(), msg.size()) <= 0) {
			logger.error("Error sending message");
		}
	}
}

void AbstractUnixSocketClient::readLoop() {
	uint32_t resp_len;
	while (running && connected) {
		ssize_t n = read(sock, &resp_len, sizeof(resp_len));
		if (n == sizeof(resp_len)) {
			resp_len = ntohl(resp_len);
			std::string data(resp_len, '\0');
			size_t total_read = 0;
			while (total_read < resp_len) {
				ssize_t r = read(sock, &data[total_read], resp_len - total_read);
				if (r <= 0) {
					logger.error(std::string(strerror(errno)));
					continue;
				}
				total_read += r;
			}

			auto json = nlohmann::json::parse(data);
			auto j	  = CommunicationMessage::from_json(json);

			if (j.type == "RESPONSE") {
				handleResponse(j);
			}
		}
	}
}

void AbstractUnixSocketClient::handleResponse(CommunicationMessage msg) {
	std::lock_guard<std::mutex> lock(mutex);
	auto it = promises.find(msg.id);
	if (it != promises.end()) {
		UnixMethodResponse wsmr{msg, ""};
		it->second.set_value(wsmr);
		promises.erase(it);
	}
}

void AbstractUnixSocketClient::handleEvent(CommunicationMessage msg) {
	std::string eventName = "unix.socket.event." + name + "." + msg.name;
	if (msg.data.empty()) {
		eventBus.emit_event(eventName);
	} else {
		eventBus.emit_event(eventName, msg.data);
	}
}

std::vector<std::any> AbstractUnixSocketClient::invoke(std::string method, std::vector<std::any> data, const int& timeout_ms) {
	CommunicationMessage cm;
	cm.type = "REQUEST";
	cm.id	= StringUtils::generateUUIDv4();
	cm.name = method;
	cm.data = data;

	std::promise<UnixMethodResponse> prom;
	std::future fut = prom.get_future();
	{
		std::lock_guard<std::mutex> lock(mutex);
		promises[cm.id] = std::move(prom);
		_message_queue.push(cm.to_json());
	}
	queue_cv.notify_one();

	if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
		logger.error("No response for {} after {} ms", timeout_ms, method);
		throw std::runtime_error("UnixSocketTimeoutError");
	}

	UnixMethodResponse resp = fut.get();

	if (!resp.error.empty()) {
		throw std::runtime_error(resp.error);
	}

	return resp.data.data;
}