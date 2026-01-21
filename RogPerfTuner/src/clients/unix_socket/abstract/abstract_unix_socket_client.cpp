#include "../../../../include/clients/unix_socket/abstract/abstract_unix_socket_client.hpp"

#include <arpa/inet.h>
#include <execinfo.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <exception>
#include <mutex>
#include <string>
#include <thread>

#include "../../../../include/utils/string_utils.hpp"
#include "../../../../include/utils/time_utils.hpp"

AbstractUnixSocketClient::AbstractUnixSocketClient(const std::string& path, const std::string& name) : Loggable(name), path(path), name(name) {
	signal(SIGPIPE, SIG_IGN);

	_running.store(true);
	_connected.store(false);

	connectionThread = std::thread([this] {
		connectionLoop();
	});
}

AbstractUnixSocketClient::~AbstractUnixSocketClient() {
	stop();
}

void AbstractUnixSocketClient::stop(bool stopConnThread) {
	try {
		Logger::add_tab();
		_connected = false;
		_running   = false;

		if (sock != -1) {
			shutdown(sock, SHUT_RDWR);
			close(sock);
			sock = -1;
		}

		queue_cv.notify_all();

		if (stopConnThread && connectionThread.joinable()) {
			connectionThread.join();
		}
		if (writeThread.joinable()) {
			writeThread.join();
		}
		if (readThread.joinable()) {
			readThread.join();
		}
		Logger::rem_tab();
	} catch (std::exception& e) {
		logger.error("Error on stop: " + std::string(e.what()));
	}
}

void AbstractUnixSocketClient::connectionLoop() {
	while (_running) {
		if (_connected) {
			TimeUtils::sleep(5000);
			continue;
		}

		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		sockaddr_un addr{};
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

		if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
			logger.debug(std::string(strerror(errno)));
			close(sock);
			sock = -1;
			TimeUtils::sleep(3000);
			continue;
		}

		eventBus.emitUnixSocketEvent(name, "connect", {});
		_connected.store(true);

		if (writeThread.joinable()) {
			writeThread.join();
		}
		writeThread = std::thread([this] {
			writeLoop();
		});

		if (readThread.joinable()) {
			readThread.join();
		}
		readThread = std::thread([this] {
			readLoop();
		});

		while (_connected) {
			for (int i = 0; i < 20 && _connected && _running; i++) {
				TimeUtils::sleep(250);
			}
			if (_connected && _running) {
				try {
					invoke("ping", {}, 100);
				} catch (std::exception& e) {
					logger.debug("Lost connection");
					stop(false);
					_running = true;
					eventBus.emitUnixSocketEvent(name, "disconnect", {});
					TimeUtils::sleep(1000);
					continue;
				}
			}
		}
	}
}

void AbstractUnixSocketClient::writeLoop() {
	while (_running && _connected) {
		std::unique_lock<std::mutex> lock(mutex);
		queue_cv.wait(lock, [this] {
			return !_message_queue.empty() || !_running;
		});

		if (!_running) {
			break;
		}

		if (!_connected) {
			lock.unlock();
			TimeUtils::sleep(50);
			continue;
		}

		if (_message_queue.empty()) {
			lock.unlock();
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
	while (_running && _connected) {
		ssize_t n = read(sock, &resp_len, sizeof(resp_len));
		if (n == sizeof(resp_len)) {
			resp_len = ntohl(resp_len);
			std::string data(resp_len, '\0');
			size_t total_read = 0;
			while (total_read < resp_len) {
				ssize_t r = read(sock, &data[total_read], resp_len - total_read);
				if (r <= 0) {
					logger.debug("Server closed the connection");
					eventBus.emitUnixSocketEvent(name, "disconnect", {});
					_connected.store(false);
					break;
				}
				total_read += r;
			}

			YAML::Node node		   = YAML::Load(data);
			CommunicationMessage j = node.as<CommunicationMessage>();

			if (j.type == "RESPONSE") {
				handleResponse(j);
			} else if (j.type == "EVENT") {
				handleEvent(j);
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
		YAML::Node node = YAML::convert<CommunicationMessage>::encode(cm);
		std::stringstream ss;
		ss << node;
		_message_queue.push(ss.str());
	}
	queue_cv.notify_one();

	if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
		if (method != "ping") {
			logger.error("No response for " + method + " after " + std::to_string(timeout_ms) + " ms");
		}
		throw std::runtime_error("UnixSocketTimeoutError");
	}

	UnixMethodResponse resp = fut.get();

	if (!resp.error.empty()) {
		throw std::runtime_error(resp.error);
	}

	return resp.data.data;
}

void AbstractUnixSocketClient::handleEvent(CommunicationMessage msg) {
	eventBus.emitUnixSocketEvent(name, msg.name, msg.data);
}

void AbstractUnixSocketClient::on_without_params(const std::string& evName, Callback&& callback) {
	on_with_params(evName, [callback = std::move(callback)](CallbackParam) {
		callback();
	});
}

void AbstractUnixSocketClient::on_with_params(const std::string& evName, CallbackWithParams&& callback) {
	eventBus.onUnixSocketEvent(name, evName, std::move(callback));
}

void AbstractUnixSocketClient::onConnect(Callback&& callback) {
	on_without_params("connect", std::move(callback));
}

void AbstractUnixSocketClient::onDisconnect(Callback&& callback) {
	on_without_params("disconnect", std::move(callback));
}

bool AbstractUnixSocketClient::connected() {
	return _connected;
}