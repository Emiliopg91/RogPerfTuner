#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <typeindex>
#include <any>
#include "../logger/logger.hpp"

typedef std::vector<std::any> CallbackParam;
typedef std::function<void(CallbackParam)> CallbackWithParams;
typedef std::function<void()> Callback;

class EventBus
{
private:
    EventBus() = default;
    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;

    std::unordered_map<std::string, std::vector<Callback>> no_params_listeners;
    std::unordered_map<std::string, std::vector<CallbackWithParams>> with_params_listeners;
    std::mutex mtx;

public:
    static EventBus &getInstance()
    {
        static EventBus instance;
        return instance;
    }

    void on_without_data(const std::string &event, Callback callback)
    {
        std::lock_guard<std::mutex> lock(mtx);
        no_params_listeners[event].emplace_back(std::move(callback));
    }

    void emit_event(const std::string &event)
    {
        std::vector<std::function<void()>> to_call;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = no_params_listeners.find(event);
            if (it == no_params_listeners.end())
                return;
            to_call = it->second;
        }

        for (auto &callback : to_call)
        {
            try
            {
                callback();
            }
            catch (const std::exception &e)
            {
            }
        }
    }

    void on_with_data(const std::string &event, CallbackWithParams &&callback)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = with_params_listeners.find(event);

        if (it == with_params_listeners.end())
        {
            auto holder = std::vector<CallbackWithParams>();
            holder.push_back(std::move(callback));
            with_params_listeners[event] = std::move(holder);
        }
        else
        {
            with_params_listeners[event].push_back(std::move(callback));
        }
    }

    void emit_event(const std::string &event, std::vector<std::any> args)
    {
        std::vector<std::function<void(std::vector<std::any>)>> to_call;

        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = with_params_listeners.find(event);
            if (it == with_params_listeners.end())
                return;

            to_call = it->second;
        }

        for (auto &callback : to_call)
        {
            try
            {
                callback(args);
            }
            catch (const std::exception &e)
            {
            }
        }
    }
};
