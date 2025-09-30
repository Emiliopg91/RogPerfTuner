#pragma once

#include <atomic>
#include <future>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "../../../events/event_bus.hpp"
#include "../../../models/others/communication_message.hpp"
#include "../../../models/others/loggable.hpp"

struct UnixMethodResponse {
	CommunicationMessage data;
	std::string error;
};

class AbstractUnixSocketClient : Loggable {
  public:
	AbstractUnixSocketClient(const std::string& path, const std::string& name);
	virtual ~AbstractUnixSocketClient();

  protected:
	std::vector<std::any> invoke(std::string method, std::vector<std::any> data, const int& timeout_ms = 3000);

  private:
	void connectionLoop();
	void writeLoop();
	void readLoop();
	void handleResponse(CommunicationMessage msg);
	void handleEvent(CommunicationMessage msg);

	std::string path;
	std::string name;
	int sock;
	std::queue<std::string> _message_queue;
	std::unordered_map<std::string, std::promise<UnixMethodResponse>> promises;

	std::atomic<bool> running;
	std::atomic<bool> connected;
	std::mutex mutex;
	std::condition_variable queue_cv;

	std::thread connectionThread;
	std::thread writeThread;
	std::thread readThread;

	EventBus& eventBus = EventBus::getInstance();
};
