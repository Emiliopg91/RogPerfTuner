#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <any>
#include <stdexcept>
#include <typeindex>

class EventBus
{
private:
    struct BaseHolder
    {
        virtual ~BaseHolder() = default;
    };

    template <typename... Args>
    struct Holder : BaseHolder
    {
        std::vector<std::function<void(Args...)>> callbacks;
    };

    std::unordered_map<std::string, std::unique_ptr<BaseHolder>> listeners;
    std::mutex mtx;

public:
    // Registrar un callback para un evento con tipo específico
    template <typename... Args>
    void on(const std::string &event, std::function<void(Args...)> callback)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = listeners.find(event);
        if (it == listeners.end())
        {
            auto holder = std::make_unique<Holder<Args...>>();
            holder->callbacks.push_back(std::move(callback));
            listeners[event] = std::move(holder);
        }
        else
        {
            auto *typed = dynamic_cast<Holder<Args...> *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Tipo de callback diferente para este evento");
            typed->callbacks.push_back(std::move(callback));
        }
    }

    template <typename... Args>
    void emit_event(const std::string &event, Args... args)
    {
        std::vector<std::function<void(Args...)>> to_call;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = listeners.find(event);
            if (it == listeners.end())
                return;
            auto *typed = dynamic_cast<Holder<Args...> *>(it->second.get());
            if (!typed)
                throw std::runtime_error("Tipo de parámetros incorrecto al emitir evento");
            to_call = typed->callbacks; // copia para ejecutar fuera del lock
        }

        for (auto &callback : to_call)
        {
            std::async(std::launch::async, callback, args...);
        }
    }
};
