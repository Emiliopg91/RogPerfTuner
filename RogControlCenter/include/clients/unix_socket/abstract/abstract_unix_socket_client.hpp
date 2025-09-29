#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <any>
#include <condition_variable>
#include <future>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>

#include "../../../events/event_bus.hpp"
#include "../../../models/others/communication_message.hpp"
#include "../../../models/others/loggable.hpp"

struct UnixMethodResponse {
	CommunicationMessage data;
	std::string error;
};

class AbstractUnixSocketClient : public Loggable {
  public:
	std::vector<std::any> invoke(const std::string& method, const std::vector<std::any>& args, const int& timeout_ms = 3000);

	void onConnect(Callback&& callback);

	void onDisconnect(Callback&& callback);

	bool connected();

	virtual ~AbstractUnixSocketClient();

  protected:
	AbstractUnixSocketClient(const std::string& socketPath, const std::string& name);

	void on_without_params(const std::string& name, Callback&& callback);

	void on_with_params(const std::string& name, CallbackWithParams&& callback);

  private:
	int _fd = -1;
	std::string _name;
	std::string _socketPath;
	bool _connected	   = false;
	EventBus& eventBus = EventBus::getInstance();
	std::thread _writeThread;
	std::thread _readThread;

	std::queue<std::string> _message_queue;
	std::mutex _queue_mutex;
	std::condition_variable _queue_cv;

	std::unordered_map<std::string, std::promise<UnixMethodResponse>> _promises;
	std::mutex _response_mutex;

	int _id_counter = 0;

	void trigger_event(const std::string& name, const std::optional<std::vector<std::any>>& data = std::nullopt);

	void handle_message(const std::string& payload);

	void listen_loop();
};
