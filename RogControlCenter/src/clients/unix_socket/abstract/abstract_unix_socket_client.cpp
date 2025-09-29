#include "../../../../include/clients/unix_socket/abstract/abstract_unix_socket_client.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <condition_variable>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include "../../../../include/events/event_bus.hpp"
#include "../../../../include/models/others/communication_message.hpp"
#include "../../../../include/utils/string_utils.hpp"

using json = nlohmann::json;

AbstractUnixSocketClient::AbstractUnixSocketClient(const std::string& socketPath, const std::string& name)
	: Loggable(name), _name(name), _socketPath(socketPath) {
	_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (_fd < 0) {
		throw std::runtime_error("Failed to create UNIX socket");
	}

	sockaddr_un addr{};
	addr.sun_family = AF_UNIX;
	std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", _socketPath.c_str());

	if (connect(_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		close(_fd);
		throw std::runtime_error("Failed to connect to UNIX socket: " + _socketPath);
	}

	_connected = true;
	trigger_event("connect");

	_writeThread = std::thread([this] {
		while (true) {
			std::unique_lock<std::mutex> lock(_queue_mutex);
			_queue_cv.wait(lock, [this] {
				return !_message_queue.empty() || !_connected;
			});

			if (!_connected) {
				break;
			}

			std::string msg = _message_queue.front();
			_message_queue.pop();
			lock.unlock();

			ssize_t n = write(_fd, msg.c_str(), msg.size());
			if (n <= 0) {
				_connected = false;
				trigger_event("disconnect");
				break;
			}
		}
	});

	_readThread = std::thread([this] {
		listen_loop();
	});
}

AbstractUnixSocketClient::~AbstractUnixSocketClient() {
	_connected = false;

	if (_fd != -1) {
		shutdown(_fd, SHUT_RDWR);
		close(_fd);
		_fd = -1;
	}

	_queue_cv.notify_all();

	if (_writeThread.joinable()) {
		_writeThread.join();
	}
	if (_readThread.joinable()) {
		_readThread.join();
	}
}

void AbstractUnixSocketClient::listen_loop() {
	while (_connected) {
		uint32_t net_len;
		ssize_t n = read(_fd, &net_len, sizeof(net_len));
		if (n <= 0) {
			_connected = false;
			trigger_event("disconnect");
			break;
		}
		if (n != sizeof(net_len)) {
			continue;
		}

		uint32_t len = ntohl(net_len);	// network -> host byte order

		std::string payload(len, '\0');
		size_t received = 0;
		while (received < len) {
			ssize_t r = read(_fd, &payload[received], len - received);
			if (r <= 0) {
				_connected = false;
				trigger_event("disconnect");
				return;
			}
			received += r;
		}

		this->handle_message(payload);
	}
}

void AbstractUnixSocketClient::trigger_event(const std::string& name, const std::optional<std::vector<std::any>>& data) {
	auto eventName = "unix." + _name + ".event." + name;
	if (data.has_value() && !data.value().empty()) {
		eventBus.emit_event(eventName, data.value());
	} else {
		eventBus.emit_event(eventName);
	}
}

void AbstractUnixSocketClient::handle_message(const std::string& payload) {
	json j = json::parse(payload, nullptr, false);
	if (j.is_discarded()) {
		std::cerr << "Failed to parse message" << std::endl;
		return;
	}

	CommunicationMessage msg = CommunicationMessage::from_json(j);

	if (msg.type == "EVENT") {
		trigger_event(msg.name, msg.data);
	} else if (msg.type == "RESPONSE") {
		std::lock_guard<std::mutex> lock(_response_mutex);
		auto it = _promises.find(msg.id);
		if (it != _promises.end()) {
			UnixMethodResponse umr{msg, ""};
			it->second.set_value(umr);
			_promises.erase(it);
		}
	}
}

std::vector<std::any> AbstractUnixSocketClient::invoke(const std::string& method, const std::vector<std::any>& args, const int& timeout_ms) {
	if (!_connected) {
		throw std::runtime_error("No connection to server");
	}

	CommunicationMessage message{"REQUEST", method, args, StringUtils::generateUUIDv4()};

	std::promise<UnixMethodResponse> prom;
	std::future<UnixMethodResponse> fut = prom.get_future();
	{
		std::lock_guard<std::mutex> lock(_response_mutex);
		_promises[message.id] = std::move(prom);
	}

	{
		std::lock_guard<std::mutex> lock(_queue_mutex);
		_message_queue.push(message.to_json());
	}
	_queue_cv.notify_one();

	if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
		logger.error("No response for {} after {} ms", timeout_ms, method);
		throw std::runtime_error("UnixSocketTimeoutError");
	}

	UnixMethodResponse resp = fut.get();

	if (!resp.error.empty()) {
		throw std::runtime_error("UnixSocketInvocationError: " + resp.error);
	}

	return resp.data.data;
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

void AbstractUnixSocketClient::on_without_params(const std::string& name, Callback&& callback) {
	auto eventName = "unix." + _name + ".event." + name;
	eventBus.on_without_data(eventName, callback);
}

void AbstractUnixSocketClient::on_with_params(const std::string& name, CallbackWithParams&& callback) {
	auto eventName = "unix." + _name + ".event." + name;
	eventBus.on_with_data(eventName, std::move(callback));
}
