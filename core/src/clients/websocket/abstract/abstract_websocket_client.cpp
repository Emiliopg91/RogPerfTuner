#include <ixwebsocket/IXWebSocket.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <chrono>
#include <nlohmann/json.hpp>

#include "../../../../include/clients/websocket/abstract/abstract_websocket_client.hpp"
#include "../../../../include/models/websocket_message.hpp"
#include "../../../../include/utils/event_bus.hpp"

using json = nlohmann::json;

AbstractWebsocketClient::AbstractWebsocketClient(const std::string &host, int port, std::string name)
    : _name(name), _host(host), _port(port), logger(Logger(name))
{
    _ws.setUrl("ws://" + _host + ":" + std::to_string(_port));

    _ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg)
                             {
            if (msg->type == ix::WebSocketMessageType::Message) {
                this->handle_message(msg->str);
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                this->_connected = true;
                trigger_event("connect");
            } else if (msg->type == ix::WebSocketMessageType::Close) {
                this->_connected = false;
                trigger_event("disconnect");
            } });

    std::thread([this]
                {
            while(true) {
                std::unique_lock<std::mutex> lock(_queue_mutex);
                _queue_cv.wait(lock, [this]{ return !_message_queue.empty(); });
                std::string msg = _message_queue.front();
                _message_queue.pop();
                lock.unlock();

                if (_connected) {
                    _ws.send(msg);
                }
            } })
        .detach();

    _ws.start();
}

void AbstractWebsocketClient::send_message(const WebsocketMessage &message)
{
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        _message_queue.push(message.to_json());
    }
    _queue_cv.notify_one();
}

// Invoke estilo Python con timeout (en milisegundos)
std::vector<std::any, std::allocator<std::any>> AbstractWebsocketClient::invoke(const std::string &method, const std::vector<std::any> &args, int timeout_ms)
{
    if (!_connected)
        throw std::runtime_error("No connection to server");

    WebsocketMessage message;
    message.type = "REQUEST";
    message.name = method;
    message.data = args;
    message.id = StringUtils::generateUUIDv4();

    WSMethodResponse resp;
    {
        std::lock_guard<std::mutex> lock(_response_mutex);
        _responses[message.id] = &resp;
    }

    send_message(message);

    auto start = std::chrono::steady_clock::now();
    while (!resp.received)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeout_ms)
        {
            std::lock_guard<std::mutex> lock(_response_mutex);
            _responses.erase(message.id);
            throw std::runtime_error("WebSocketTimeoutError");
        }
    }

    if (!resp.error.empty())
        throw std::runtime_error("WebSocketInvocationError: " + resp.error);

    return resp.data.data;
}

void AbstractWebsocketClient::trigger_event(const std::string &name, std::optional<std::vector<std::any>> data)
{
    auto eventName = "ws." + _name + ".event." + name;
    if (data.has_value() && !data.value().empty())
    {
        EventBus::getInstance().emit_event(eventName, std::move(data.value()));
    }
    else
    {
        EventBus::getInstance().emit_event(eventName);
    }
}

void AbstractWebsocketClient::handle_message(const std::string &payload)
{
    json j = json::parse(payload, nullptr, false);
    if (j.is_discarded())
    {
        std::cerr << "Failed to parse message" << std::endl;
        return;
    }

    WebsocketMessage msg = WebsocketMessage::from_json(j);

    if (msg.type == "EVENT")
    {
        trigger_event(msg.name, msg.data);
    }
    else if (msg.type == "RESPONSE")
    {
        std::lock_guard<std::mutex> lock(_response_mutex);
        if (_responses.find(msg.id) != _responses.end())
        {
            auto *resp = _responses[msg.id];
            resp->data = msg;
            resp->received = true;
            _responses.erase(msg.id);
        }
    }
}