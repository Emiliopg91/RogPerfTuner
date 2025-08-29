#pragma once

#include "../logger/logger.hpp"
#include "../clients/websocket/steam_client.hpp"

class SteamService
{
private:
    Logger logger{"SteamService"};
    std::map<int, std::string> runningGames;

    SteamService()
    {
        logger.info("Initializing SteamClient");
        logger.add_tab();
        SteamClient::getInstance().onConnect([]()
                                             { std::cout << "Cliente iniciado" << std::endl; });
        logger.rem_tab();
    };

public:
    static SteamService &getInstance()
    {
        static SteamService instance;
        return instance;
    }

    const std::map<int, std::string> &getRunningGames() const;
};