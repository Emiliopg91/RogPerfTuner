#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "framework/abstracts/singleton.hpp"

typedef std::vector<std::any> CallbackParam;
typedef std::any CallbackAnyParam;
typedef std::function<void(CallbackParam)> CallbackWithParams;
typedef std::function<void(CallbackAnyParam)> CallbackWithAnyParam;
typedef std::function<void()> Callback;

class EventBus : public Singleton<EventBus> {
  public:
	void on_without_data(const std::string_view& event, Callback callback);
	void emit_event(const std::string_view& event);
	void on_with_data(const std::string_view& event, CallbackWithParams&& callback);
	void emit_event(const std::string_view& event, const std::vector<std::any>& args);

	void on_without_data(const std::string& event, Callback callback);
	void emit_event(const std::string& event);
	void on_with_data(const std::string& event, CallbackWithParams&& callback);
	void emit_event(const std::string& event, const std::vector<std::any>& args);

  private:
	EventBus()							 = default;
	EventBus(const EventBus&)			 = delete;
	EventBus& operator=(const EventBus&) = delete;

	std::unordered_map<std::string, std::vector<Callback>> no_params_listeners;
	std::unordered_map<std::string, std::vector<CallbackWithParams>> with_params_listeners;
	std::mutex mtx;

	friend class Singleton<EventBus>;
};
