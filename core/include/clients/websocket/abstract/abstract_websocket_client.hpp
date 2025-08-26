#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>

#include "RccCommons.hpp"

using json = nlohmann::json;

class WebSocketClient
{
private:
    std::string host;
    int port;
    ix::WebSocket ws;
    std::thread runner;
    std::thread sender_thread;
    std::queue<std::string> message_queue;
    std::mutex queue_mtx;
    std::condition_variable queue_cv;
    std::atomic<bool> running{true};
    std::atomic<bool> connected{false};

    std::unordered_map<std::string, std::function<void(const json &)>> event_handlers;
    std::unordered_map<std::string, std::queue<json>> response_map;
    Logger logger{"WebSocketClient"};

public:
    WebSocketClient(std::string host_, int port_)
        : host(host_), port(port_)
    {
        ws.setUrl("ws://" + host + ":" + std::to_string(port));

        ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg)
                                {
            if (msg->type == ix::WebSocketMessageType::Open)
            {
                connected = true;
                logger.info("Connected to WebSocket server");
            }
            else if (msg->type == ix::WebSocketMessageType::Close)
            {
                connected = false;
                logger.info("Disconnected from WebSocket server");
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                connected = false;
                logger.info("WebSocket error: " + msg->errorInfo.reason);
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                try
                {
                    auto j = json::parse(msg->str);
                    std::string type = j.value("type", "");
                    if (type == "EVENT")
                    {
                        auto name = j["name"].get<std::string>();
                        if (event_handlers.count(name))
                            event_handlers[name](j["data"]);
                    }
                    else if (type == "RESPONSE")
                    {
                        auto id = j["id"].get<std::string>();
                        response_map[id].push(j);
                    }
                }
                catch (...)
                {
                    logger.info("Failed to parse incoming message");
                }
            } });

        runner = std::thread([this]()
                             { ws.start(); });

        sender_thread = std::thread([this]()
                                    { this->message_sender(); });
    }

    ~WebSocketClient()
    {
        running = false;
        ws.stop();
        if (runner.joinable())
            runner.join();
        queue_cv.notify_all();
        if (sender_thread.joinable())
            sender_thread.join();
    }

    void message_sender()
    {
        while (running)
        {
            std::unique_lock<std::mutex> lock(queue_mtx);
            queue_cv.wait(lock, [this]
                          { return !message_queue.empty() || !running; });

            while (!message_queue.empty())
            {
                auto msg = message_queue.front();
                message_queue.pop();
                lock.unlock();

                if (connected)
                    ws.send(msg);

                lock.lock();
            }
        }
    }

    void send_message(const json &msg)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mtx);
            message_queue.push(msg.dump());
        }
        queue_cv.notify_one();
    }

    json invoke(const std::string &method, const json &args, int timeout_ms = 3000)
    {
        if (!connected)
            throw std::runtime_error("No connection");

        json msg = {
            {"type", "REQUEST"},
            {"id", std::to_string(rand())}, // simple id
            {"name", method},
            {"data", args}};
        std::string id = msg["id"];

        response_map[id] = std::queue<json>();
        send_message(msg);

        auto t0 = std::chrono::steady_clock::now();
        while (true)
        {
            if (!response_map[id].empty())
            {
                json resp = response_map[id].front();
                response_map[id].pop();
                response_map.erase(id);
                return resp["data"];
            }

            if (std::chrono::steady_clock::now() - t0 > std::chrono::milliseconds(timeout_ms))
            {
                response_map.erase(id);
                throw std::runtime_error("Timeout waiting for response");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void on(const std::string &event, std::function<void(const json &)> callback)
    {
        event_handlers[event] = callback;
    }
};
