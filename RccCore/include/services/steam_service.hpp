#pragma once

#include "RccCommons.hpp"
#include "../models/hardware/gpu_brand.hpp"
#include "../models/steam/mangohud_level.hpp"
#include "../models/settings/root_config.hpp"
#include "../models/steam/wine_sync_option.hpp"
#include "../clients/websocket/steam_client.hpp"

class SteamService
{
private:
    inline static std::string WRAPPER_PATH = Constants::BIN_DIR + "/steam/run";
    Logger logger{"SteamService"};
    std::map<unsigned int, std::string> runningGames;
    bool rccdcEnabled = false;
    std::thread installer;

    SteamService();

    void onConnect(bool onBoot = false);
    void onDisconnect();
    void onGameLaunch(unsigned int gid, std::string name, int pid);
    void onFirstGameRun(unsigned int gid, std::string name, std::map<std::string, std::string> env);
    void onGameStop(unsigned int gid, std::string name);
    void setProfileForGames(bool onConnect = false);
    void installRccDC();
    void copyPlugin();
    bool checkIfRequiredInstallation();

public:
    static SteamService &getInstance()
    {
        static SteamService instance;
        return instance;
    }

    const std::map<unsigned int, std::string> &getRunningGames() const;
    const std::map<std::string, GameEntry> &getGames() const;

    bool isRunning(unsigned int appid) const;
    bool metricsEnabled();
    SteamGameConfig getConfiguration(std::string gid);

    std::optional<GpuBrand> getPreferedGpu(unsigned int gid);
    void setPreferedGpu(unsigned int gid, std::optional<GpuBrand> gpu);

    bool isSteamDeck(unsigned int gid);
    void setSteamDeck(unsigned int gid, bool value);

    MangoHudLevel getMetricsLevel(unsigned int gid);
    void setMetricsLevel(unsigned int gid, MangoHudLevel level);

    WineSyncOption getWineSync(unsigned int gid);
    void setWineSync(unsigned int gid, WineSyncOption level);

    bool isProton(unsigned int gid);

    std::string getEnvironment(unsigned int gid);
    void setEnvironment(unsigned int gid, std::string env);

    std::string getParameters(unsigned int gid);
    void setParameters(unsigned int gid, std::string env);
};