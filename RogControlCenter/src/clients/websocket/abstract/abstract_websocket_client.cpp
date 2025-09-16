#include "../../../../include/clients/websocket/abstract/abstract_websocket_client.hpp"

#include <ixwebsocket/IXWebSocket.h>

#include <condition_variable>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <thread>
#include <unordered_map>

#include "../../../../include/events/event_bus.hpp"
#include "../../../../include/models/others/websocket_message.hpp"
#include "../../../../include/utils/string_utils.hpp"

using json = nlohmann::json;

AbstractWebsocketClient::AbstractWebsocketClient(const std::string& host, const int& port, const std::string& name)
	: _name(name), _host(host), _port(port), logger(Logger(name)) {
	_ws.setUrl("ws://" + _host + ":" + std::to_string(_port));

	_ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
		if (msg->type == ix::WebSocketMessageType::Message) {
			this->handle_message(msg->str);
		} else if (msg->type == ix::WebSocketMessageType::Open) {
			this->_connected = true;
			trigger_event("connect");
		} else if (msg->type == ix::WebSocketMessageType::Close) {
			this->_connected = false;
			trigger_event("disconnect");
		}
	});

	std::thread([this] {
		while (true) {
			std::unique_lock<std::mutex> lock(_queue_mutex);
			_queue_cv.wait(lock, [this] {
				return !_message_queue.empty();
			});
			std::string msg = _message_queue.front();
			_message_queue.pop();
			lock.unlock();

			if (_connected) {
				_ws.send(msg);
			}
		}
	}).detach();

	_ws.start();
}

void AbstractWebsocketClient::trigger_event(const std::string& name, const std::optional<std::vector<std::any>>& data) {
	auto eventName = "ws." + _name + ".event." + name;
	if (data.has_value() && !data.value().empty()) {
		eventBus.emit_event(eventName, data.value());
	} else {
		eventBus.emit_event(eventName);
	}
}

void AbstractWebsocketClient::handle_message(const std::string& payload) {
	json j = json::parse(payload, nullptr, false);
	if (j.is_discarded()) {
		std::cerr << "Failed to parse message" << std::endl;
		return;
	}

	WebsocketMessage msg = WebsocketMessage::from_json(j);

	if (msg.type == "EVENT") {
		trigger_event(msg.name, msg.data);
	} else if (msg.type == "RESPONSE") {
		std::lock_guard<std::mutex> lock(_response_mutex);
		auto it = _promises.find(msg.id);
		if (it != _promises.end()) {
			WSMethodResponse wsmr{msg, ""};
			it->second.set_value(wsmr);
			_promises.erase(it);
		}
	}
}

std::vector<std::any> AbstractWebsocketClient::invoke(const std::string& method, const std::vector<std::any>& args, const int& timeout_ms) {
	if (!_connected) {
		throw std::runtime_error("No connection to server");
	}

	WebsocketMessage message{"REQUEST", method, args, StringUtils::generateUUIDv4()};

	std::promise<WSMethodResponse> prom;
	std::future fut = prom.get_future();
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
		throw std::runtime_error("WebSocketTimeoutError");
	}

	WSMethodResponse resp = fut.get();

	if (!resp.error.empty()) {
		throw std::runtime_error("WebSocketInvocationError: " + resp.error);
	}

	return resp.data.data;
}

void AbstractWebsocketClient::onConnect(Callback&& callback) {
	on_without_params("connect", std::move(callback));
}

void AbstractWebsocketClient::onDisconnect(Callback&& callback) {
	on_without_params("disconnect", std::move(callback));
}

bool AbstractWebsocketClient::connected() {
	return _connected;
}

void AbstractWebsocketClient::on_without_params(const std::string& name, Callback&& callback) {
	auto eventName = "ws." + _name + ".event." + name;
	eventBus.on_without_data(eventName, callback);
}

void AbstractWebsocketClient::on_with_params(const std::string& name, CallbackWithParams&& callback) {
	auto eventName = "ws." + _name + ".event." + name;
	eventBus.on_with_data(eventName, std::move(callback));
}