#pragma once

#include <atomic>
#include <future>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "abstracts/loggable.hpp"
#include "models/others/communication_message.hpp"
#include "utils/event_bus_wrapper.hpp"

struct UnixMethodResponse {
	CommunicationMessage data;
	std::string error;
};

class AbstractUnixSocketClient : Loggable {
  public:
	AbstractUnixSocketClient(const std::string& path, const std::string& name);
	virtual ~AbstractUnixSocketClient();

	bool connected();
	void onConnect(Callback&& callback);
	void onDisconnect(Callback&& callback);

  protected:
	std::vector<std::any> invoke(std::string method, std::vector<std::any> data, const int& timeout_ms = 3000);

	void on_without_params(const std::string& name, Callback&& callback);

	void on_with_params(const std::string& name, CallbackWithParams&& callback);

  private:
	void connectionLoop();
	void writeLoop();
	void readLoop();
	void handleResponse(CommunicationMessage msg);
	void handleEvent(CommunicationMessage msg);
	void stop(bool stopConnThread = true);

	std::string path;
	std::string name;
	int sock;
	std::queue<std::string> _message_queue;
	std::unordered_map<std::string, std::promise<UnixMethodResponse>> promises;

	std::atomic<bool> _running;
	std::atomic<bool> _connected;
	std::mutex mutex;
	std::condition_variable queue_cv;

	std::thread connectionThread;
	std::thread writeThread;
	std::thread readThread;

	EventBusWrapper& eventBus = EventBusWrapper::getInstance();
};
