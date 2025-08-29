#pragma once

#include "RccCommons.hpp"
#include "abstract/abstract_websocket_client.hpp"

class SteamClient : public AbstractWebsocketClient
{
private:
    SteamClient() : AbstractWebsocketClient("localhost", Constants::WS_PORT, "SteamClient")
    {
    }

public:
    static SteamClient &getInstance()
    {
        static SteamClient instance;
        return instance;
    }
};