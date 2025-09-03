#pragma once

#include <any>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "RccCommons.hpp"
#include "events.hpp"

typedef std::vector<std::any> CallbackParam;
typedef std::function<void(CallbackParam)> CallbackWithParams;
typedef std::function<void()> Callback;

class EventBus {
  private:
	EventBus()							 = default;
	EventBus(const EventBus&)			 = delete;
	EventBus& operator=(const EventBus&) = delete;

	std::unordered_map<std::string, std::vector<Callback>> no_params_listeners;
	std::unordered_map<std::string, std::vector<CallbackWithParams>> with_params_listeners;
	std::mutex mtx;

  public:
	static EventBus& getInstance() {
		static EventBus instance;
		return instance;
	}

	void on_without_data(const std::string& event, Callback callback);

	void emit_event(const std::string& event);

	void on_with_data(const std::string& event, CallbackWithParams&& callback);

	void emit_event(const std::string& event, const std::vector<std::any>& args);
};
