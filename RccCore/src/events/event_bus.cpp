#include "../../include/events/event_bus.hpp"

#include <any>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "RccCommons.hpp"

void EventBus::on_without_data(const std::string& event, Callback callback) {
	std::lock_guard<std::mutex> lock(mtx);
	no_params_listeners[event].emplace_back(std::move(callback));
}

void EventBus::emit_event(const std::string& event) {
	std::vector<std::function<void()>> to_call;
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = no_params_listeners.find(event);
		if (it == no_params_listeners.end())
			return;
		to_call = it->second;
	}

	for (auto& callback : to_call) {
		try {
			callback();
		} catch (const std::exception& e) {
		}
	}
}

void EventBus::on_with_data(const std::string& event, CallbackWithParams&& callback) {
	std::lock_guard<std::mutex> lock(mtx);
	auto it = with_params_listeners.find(event);

	if (it == with_params_listeners.end()) {
		auto holder = std::vector<CallbackWithParams>();
		holder.push_back(std::move(callback));
		with_params_listeners[event] = std::move(holder);
	} else {
		with_params_listeners[event].push_back(std::move(callback));
	}
}

void EventBus::emit_event(const std::string& event, const std::vector<std::any>& args) {
	std::vector<std::function<void(std::vector<std::any>)>> to_call;

	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = with_params_listeners.find(event);
		if (it == with_params_listeners.end())
			return;

		to_call = it->second;
	}

	for (auto& callback : to_call) {
		try {
			callback(args);
		} catch (const std::exception& e) {
		}
	}
}