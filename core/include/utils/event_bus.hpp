#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <stdexcept>

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
    };

    struct Holder : BaseHolder
    {
        std::vector<std::function<void()>> callbacks;
    };

    std::unordered_map<std::string, std::unique_ptr<BaseHolder>> listeners;
    std::mutex mtx;

public:
    static EventBus &getInstance()
    {
        static EventBus instance;
        return instance;
    }

    template <typename Callback>
    void on(const std::string &event, Callback &&callback)
    {
        auto func = [cb = std::forward<Callback>(callback)]()
        {
            cb();
        };

        std::lock_guard<std::mutex> lock(mtx);

        auto it = listeners.find(event);
        if (it == listeners.end())
        {
            auto holder = std::make_unique<Holder>();
            holder->callbacks.push_back(std::move(func));
            listeners[event] = std::move(holder);
        }
        else
        {
            auto *typed = dynamic_cast<Holder *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Tipo de callback diferente para este evento");
            typed->callbacks.push_back(std::move(func));
        }
    }

    void emit_async(const std::string &event)
    {
        std::vector<std::function<void()>> to_call;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = listeners.find(event);
            if (it == listeners.end())
                return;
            auto *typed = dynamic_cast<Holder *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Invalid holder");
            to_call = typed->callbacks;
        }

        for (auto &callback : to_call)
        {
            std::async(std::launch::async, callback);
        }
    }

    void emit_sequential(const std::string &event)
    {
        std::vector<std::function<void()>> to_call;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = listeners.find(event);
            if (it == listeners.end())
                return;

            auto *typed = dynamic_cast<Holder *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Invalid holder");

            to_call = typed->callbacks;
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
};
