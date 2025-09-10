#pragma once

#include <ixwebsocket/IXWebSocket.h>

#include <condition_variable>
#include <future>
#include <mutex>
#include <nlohmann/json.hpp>  // Manejo de JSON
#include <queue>
#include <unordered_map>

#include "../../../events/event_bus.hpp"
#include "../../../logger/logger.hpp"
#include "../../../models/others/websocket_message.hpp"

using json = nlohmann::json;

struct WSMethodResponse {
	WebsocketMessage data;
	std::string error;
};

class AbstractWebsocketClient {
  public:
	AbstractWebsocketClient(const std::string& host, const int& port, const std::string& name);

	std::vector<std::any> invoke(const std::string& method, const std::vector<std::any>& args, const int& timeout_ms = 3000);

	void onConnect(Callback&& callback) {
		on_without_params("connect", std::move(callback));
	}

	void onDisconnect(Callback&& callback) {
		on_without_params("disconnect", std::move(callback));
	}

	bool connected() {
		return _connected;
	}

  protected:
	void on_without_params(const std::string& name, Callback&& callback) {
		auto eventName = "ws." + _name + ".event." + name;
		eventBus.on_without_data(eventName, callback);
	}

	void on_with_params(const std::string& name, CallbackWithParams&& callback) {
		auto eventName = "ws." + _name + ".event." + name;
		eventBus.on_with_data(eventName, std::move(callback));
	}

  private:
	ix::WebSocket _ws;
	std::string _name;
	std::string _host;
	int _port;
	bool _connected = false;
	Logger logger;
	EventBus& eventBus = EventBus::getInstance();

	std::queue<std::string> _message_queue;
	std::mutex _queue_mutex;
	std::condition_variable _queue_cv;

	std::unordered_map<std::string, std::promise<WSMethodResponse>> _promises;
	std::mutex _response_mutex;

	int _id_counter = 0;

	void trigger_event(const std::string& name, const std::optional<std::vector<std::any>>& data = std::nullopt);

	void handle_message(const std::string& payload);
};
