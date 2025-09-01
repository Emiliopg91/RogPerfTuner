#pragma once

#include "RccCommons.hpp"
#include "../clients/websocket/steam_client.hpp"

class SteamService
{
private:
    inline static std::string WRAPPER_PATH = Constants::BIN_DIR + "/steam/run";
    Logger logger{"SteamService"};
    std::map<unsigned int, std::string> runningGames;

    SteamService();

    void onConnect(bool onBoot = false);
    void onDisconnect();
    void onGameLaunch(unsigned int gid, std::string name, int pid);
    void onFirstGameRun(unsigned int gid, std::string name, std::map<std::string, std::string> env);
    void onGameStop(unsigned int gid, std::string name);
    void setProfileForGames(bool onConnect = false);

public:
    static SteamService &getInstance()
    {
        static SteamService instance;
        return instance;
    }

    const std::map<unsigned int, std::string> &getRunningGames() const;
    SteamGameConfig getConfiguration(std::string gid);
};