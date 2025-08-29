#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <typeindex>
#include "../logger/logger.hpp"

class EventBus
{
private:
    EventBus() = default;
    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;

    struct BaseHolder
    {
        virtual ~BaseHolder() = default;
        virtual std::type_index getType() const = 0;
    };

    template <typename... Args>
    struct Holder : BaseHolder
    {
        std::vector<std::function<void(Args...)>> callbacks;

        std::type_index getType() const override
        {
            return typeid(Holder<Args...>);
        }
    };

    std::unordered_map<std::string, std::vector<std::function<void()>>> no_params_listeners;
    std::unordered_map<std::string, std::unique_ptr<BaseHolder>> with_params_listeners;
    std::mutex mtx;

public:
    static EventBus &getInstance()
    {
        static EventBus instance;
        return instance;
    }

    void on_without_data(const std::string &event, std::function<void()> callback)
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

    template <typename... Args, typename Callback>
    void on_with_data(const std::string &event, Callback &&callback)
    {
        using FuncType = std::function<void(Args...)>;
        auto func = FuncType(std::forward<Callback>(callback));

        std::lock_guard<std::mutex> lock(mtx);
        auto it = with_params_listeners.find(event);

        using HolderType = Holder<Args...>;

        if (it == with_params_listeners.end())
        {
            auto holder = std::make_unique<HolderType>();
            holder->callbacks.push_back(std::move(func));
            with_params_listeners[event] = std::move(holder);
        }
        else
        {
            auto *typed = dynamic_cast<HolderType *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Invalid holder");
            typed->callbacks.push_back(std::move(func));
        }
    }

    template <typename... Args>
    void emit_event(const std::string &event, Args... args)
    {
        using HolderType = Holder<Args...>;
        std::vector<std::function<void(Args...)>> to_call;

        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = with_params_listeners.find(event);
            if (it == with_params_listeners.end())
                return;

            auto *typed = dynamic_cast<HolderType *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Invalid holder");
            to_call = typed->callbacks;
        }

        for (auto &callback : to_call)
        {
            try
            {
                callback(args...);
            }
            catch (const std::exception &e)
            {
            }
        }
    }
};
