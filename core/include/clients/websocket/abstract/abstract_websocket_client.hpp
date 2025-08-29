#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <chrono>
#include <nlohmann/json.hpp> // Manejo de JSON

#include "../../../logger/logger.hpp"
#include "../../../models/websocket_message.hpp"
#include "../../../utils/event_bus.hpp"

using json = nlohmann::json;

struct WSMethodResponse
{
    WebsocketMessage data;
    std::string error;
    bool received = false;
};

class AbstractWebsocketClient
{
public:
    AbstractWebsocketClient(const std::string &host, int port, std::string name);

    void send_message(const WebsocketMessage &message);

    std::vector<std::any> invoke(const std::string &method, const std::vector<std::any> &args, int timeout_ms = 3000);

    template <typename Callback>
    void onConnect(Callback callback)
    {
        on("connect", callback);
    }

protected:
    template <typename... Args, typename Callback>
    void on(const std::string &name, Callback &&callback)
    {
        auto eventName = "ws." + _name + ".event." + name;
        if constexpr (sizeof...(Args) == 0)
        {
            EventBus::getInstance().on_without_data(eventName, callback);
        }
        else
        {
            EventBus::getInstance().on_with_data(eventName, callback);
        }
    }

private:
    ix::WebSocket _ws;
    std::string _name;
    std::string _host;
    int _port;
    bool _connected = false;
    Logger logger;

    std::queue<std::string> _message_queue;
    std::mutex _queue_mutex;
    std::condition_variable _queue_cv;

    std::unordered_map<std::string, WSMethodResponse *> _responses;
    std::mutex _response_mutex;

    int _id_counter = 0;

    void trigger_event(const std::string &name, std::optional<std::vector<std::any>> data = std::nullopt);

    void handle_message(const std::string &payload);
};
