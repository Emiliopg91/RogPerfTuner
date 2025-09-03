#pragma once

#include "RccCommons.hpp"
#include "../../models/steam/steam_game_details.hpp"
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

    void onGameLaunch(CallbackWithParams &&callback)
    {
        on_with_params("launch_game", std::move(callback));
    }

    void onGameStop(CallbackWithParams &&callback)
    {
        on_with_params("stop_game", std::move(callback));
    }

    const std::vector<SteamGameDetails> getAppsDetails(const std::vector<unsigned int> &appIds)
    {
        std::vector<std::any> converted;
        converted.reserve(appIds.size());
        for (auto val : appIds)
        {
            converted.emplace_back(val);
        }

        std::vector<SteamGameDetails> result;

        auto invResult = invoke("get_apps_details", converted);

        auto map = (std::any_cast<json>(invResult[0])).get<std::map<std::string, SteamGameDetails>>();
        for (const auto &[key, val] : map)
        {
            result.emplace_back(val);
        }

        return result;
    }

    void setLaunchOptions(const int &appid, const std::string &launchOpts)
    {
        invoke("set_launch_options", {appid, launchOpts});
    }

    std::string getIcon(const int &appid)
    {
        return std::any_cast<std::string>(invoke("get_icon", {appid})[0]);
    }
};