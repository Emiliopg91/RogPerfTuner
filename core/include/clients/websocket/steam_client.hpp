#pragma once

#include "RccCommons.hpp"
#include "../../models/steam_game_details.hpp"
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

    std::vector<SteamGameDetails> getAppsDetails(std::vector<unsigned int> appIds)
    {
        std::vector<std::any> converted;
        converted.reserve(appIds.size());
        for (auto val : appIds)
        {
            converted.emplace_back(val);
        }
        std::vector<SteamGameDetails> result;

        auto invResult = invoke("get_apps_details", converted);

        json j = std::any_cast<json>(invResult[0]);
        for (auto e : j.items())
        {
            result.emplace_back(SteamGameDetails::from_json(e.value()));
        }

        return result;
    }

    void setLaunchOptions(int appid, std::string launchOpts)
    {
        invoke("set_launch_options", {appid, launchOpts});
    }
};