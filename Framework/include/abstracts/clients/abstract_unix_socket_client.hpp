#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <yaml-cpp/yaml.h>

#include <atomic>
#include <future>
#include <mutex>
#include <queue>
#include <regex>
#include <unordered_map>

#include "abstracts/loggable.hpp"
#include "events/event_bus.hpp"

struct UnixCommunicationMessage {
	std::string type;
	std::string name;
	std::vector<std::any> data = {};
	std::string id;
	std::optional<std::string> error = std::nullopt;
};

struct UnixMethodResponse {
	UnixCommunicationMessage data;
	std::string error;
};

namespace YAML {
template <>
struct convert<UnixCommunicationMessage> {
	static Node encode(const UnixCommunicationMessage& msg) {
		Node node;
		node["type"] = msg.type;
		node["name"] = msg.name;

		Node dataNode;
		for (const auto& elem : msg.data) {
			if (elem.type() == typeid(std::string)) {
				dataNode.push_back(std::any_cast<std::string>(elem));
			} else if (elem.type() == typeid(const char*)) {
				dataNode.push_back(std::string(std::any_cast<const char*>(elem)));
			} else if (elem.type() == typeid(int)) {
				dataNode.push_back(std::any_cast<int>(elem));
			} else if (elem.type() == typeid(uint32_t)) {
				dataNode.push_back(static_cast<uint32_t>(std::any_cast<uint32_t>(elem)));
			} else if (elem.type() == typeid(double)) {
				dataNode.push_back(std::any_cast<double>(elem));
			} else if (elem.type() == typeid(bool)) {
				dataNode.push_back(std::any_cast<bool>(elem));
			} else {
				dataNode.push_back("<unsupported>");
			}
		}
		node["data"] = dataNode;

		node["id"] = msg.id;

		if (msg.error.has_value()) {
			node["error"] = *msg.error;
		}

		return node;
	}

	static bool decode(const Node& node, UnixCommunicationMessage& msg) {
		if (!node.IsMap()) {
			return false;
		}

		msg.type = node["type"] ? node["type"].as<std::string>() : "";
		msg.name = node["name"] ? node["name"].as<std::string>() : "";
		msg.id	 = node["id"] ? node["id"].as<std::string>() : "";

		msg.data.clear();
		if (node["data"] && node["data"].IsSequence()) {
			for (const auto& item : node["data"]) {
				if (item.IsScalar()) {
					std::string s = item.Scalar();

					if (item.Tag() == "!!int") {
						msg.data.push_back(item.as<int64_t>());
					} else if (item.Tag() == "!!float") {
						msg.data.push_back(item.as<double>());
					} else if (item.Tag() == "!!bool") {
						msg.data.push_back(item.as<bool>());
					} else {
						if (std::regex_match(s, std::regex("^-?[0-9]+$"))) {
							try {
								msg.data.push_back(std::stoll(s));
							} catch (std::exception& e) {
								msg.data.push_back(static_cast<uint64_t>(std::stoull(s)));
							}
						} else if (std::regex_match(s, std::regex("^[0-9]+u$"))) {
							msg.data.push_back(static_cast<uint64_t>(std::stoull(s)));
						} else if (std::regex_match(s, std::regex("^-?[0-9]*\\.[0-9]+$"))) {
							msg.data.push_back(std::stod(s));
						} else if (s == "true" || s == "false") {
							msg.data.push_back(s == "true");
						} else {
							msg.data.push_back(s);
						}
					}
				} else if (item.IsMap()) {
					std::stringstream ss;
					ss << item;
					msg.data.push_back(ss.str());
				} else if (item.IsSequence()) {
					std::stringstream ss;
					ss << item;
					msg.data.push_back(ss.str());
				}
			}
		}

		if (node["error"] && !node["error"].IsNull()) {
			msg.error = node["error"].as<std::string>();
		} else {
			msg.error.reset();
		}

		return true;
	}
};
}  // namespace YAML

class AbstractUnixSocketClient : Loggable {
  public:
	void emitUnixSocketEvent(EventBus& eventBus, std::string name, std::string event, CallbackParam value);

	void onUnixSocketEvent(EventBus& eventBus, std::string name, std::string event, CallbackWithParams&& callback);

	AbstractUnixSocketClient(const std::string& path, const std::string& name);

	~AbstractUnixSocketClient();

	void onConnect(Callback&& callback);

	void onDisconnect(Callback&& callback);

	bool connected();

  protected:
	std::vector<std::any> invoke(std::string method, std::vector<std::any> data, const int& timeout_ms = 3000);

	void on_without_params(const std::string& evName, Callback&& callback);

	void on_with_params(const std::string& evName, CallbackWithParams&& callback);

  private:
	void connectionLoop();

	void writeLoop();

	void readLoop();

	void handleResponse(UnixCommunicationMessage msg);

	void handleEvent(UnixCommunicationMessage msg);

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

	EventBus& eventBus = EventBus::getInstance();
};