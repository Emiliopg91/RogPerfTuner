#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <yaml-cpp/yaml.h>

#include <atomic>
#include <csignal>
#include <future>
#include <mutex>
#include <queue>
#include <regex>
#include <unordered_map>

#include "abstracts/loggable.hpp"
#include "events/event_bus.hpp"
#include "string_utils.hpp"
#include "time_utils.hpp"

struct UnixCommunicationMessage {
	std::string type;
	std::string name;
	std::vector<std::any> data = {};
	std::string id;
	std::optional<std::string> error = std::nullopt;
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

struct UnixMethodResponse {
	UnixCommunicationMessage data;
	std::string error;
};

class AbstractUnixSocketClient : Loggable {
  public:
	void emitUnixSocketEvent(EventBus& eventBus, std::string name, std::string event, CallbackParam value) {
		eventBus.emit_event("unix.socket." + name + ".event." + event, value);
	}

	void onUnixSocketEvent(EventBus& eventBus, std::string name, std::string event, CallbackWithParams&& callback) {
		eventBus.on_with_data("unix.socket." + name + ".event." + event, [cb = std::move(callback)](CallbackParam data) {
			cb(data);
		});
	}

	AbstractUnixSocketClient(const std::string& path, const std::string& name) : Loggable(name), path(path), name(name) {
		signal(SIGPIPE, SIG_IGN);

		_running.store(true);
		_connected.store(false);

		connectionThread = std::thread([this] {
			connectionLoop();
		});
	}

	~AbstractUnixSocketClient() {
		stop();
	}

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
	std::vector<std::any> invoke(std::string method, std::vector<std::any> data, const int& timeout_ms = 3000) {
		UnixCommunicationMessage cm;
		cm.type = "REQUEST";
		cm.id	= StringUtils::generateUUIDv4();
		cm.name = method;
		cm.data = data;

		std::promise<UnixMethodResponse> prom;
		std::future fut = prom.get_future();
		{
			std::lock_guard<std::mutex> lock(mutex);
			promises[cm.id] = std::move(prom);
			YAML::Node node = YAML::convert<UnixCommunicationMessage>::encode(cm);
			std::stringstream ss;
			ss << node;
			_message_queue.push(ss.str());
		}
		queue_cv.notify_one();

		if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
			if (method != "ping") {
				logger->error("No response for {} after {} ms", method, timeout_ms);
			}
			throw std::runtime_error("UnixSocketTimeoutError");
		}

		UnixMethodResponse resp = fut.get();

		if (!resp.error.empty()) {
			throw std::runtime_error(resp.error);
		}

		return resp.data.data;
	}

	void on_without_params(const std::string& evName, Callback&& callback) {
		on_with_params(evName, [callback = std::move(callback)](CallbackParam) {
			callback();
		});
	}

	void on_with_params(const std::string& evName, CallbackWithParams&& callback) {
		onUnixSocketEvent(eventBus, name, evName, std::move(callback));
	}

  private:
	void connectionLoop() {
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
				logger->debug(std::string(strerror(errno)));
				close(sock);
				sock = -1;
				TimeUtils::sleep(3000);
				continue;
			}

			emitUnixSocketEvent(eventBus, name, "connect", {});
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
						logger->debug("Lost connection");
						stop(false);
						_running = true;
						emitUnixSocketEvent(eventBus, name, "disconnect", {});
						TimeUtils::sleep(1000);
						continue;
					}
				}
			}
		}
	}

	void writeLoop() {
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
				logger->error("Error sending message");
			}
		}
	}

	void readLoop() {
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
						logger->debug("Server closed the connection");
						emitUnixSocketEvent(eventBus, name, "disconnect", {});
						_connected.store(false);
						break;
					}
					total_read += r;
				}

				YAML::Node node			   = YAML::Load(data);
				UnixCommunicationMessage j = node.as<UnixCommunicationMessage>();

				if (j.type == "RESPONSE") {
					handleResponse(j);
				} else if (j.type == "EVENT") {
					handleEvent(j);
				}
			}
		}
	}

	void handleResponse(UnixCommunicationMessage msg) {
		std::lock_guard<std::mutex> lock(mutex);
		auto it = promises.find(msg.id);
		if (it != promises.end()) {
			UnixMethodResponse wsmr{msg, ""};
			it->second.set_value(wsmr);
			promises.erase(it);
		}
	}

	void handleEvent(UnixCommunicationMessage msg) {
		emitUnixSocketEvent(eventBus, name, msg.name, msg.data);
	}

	void stop(bool stopConnThread = true) {
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
			logger->error("Error on stop: {}", e.what());
		}
	}

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