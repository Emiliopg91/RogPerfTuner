#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct SteamGameDetails
{
    int appid;
    std::string name;
    bool is_steam_app = true;
    std::string launch_opts;

    // from_json
    inline static SteamGameDetails from_json(const json &j)
    {
        SteamGameDetails g;
        g.appid = j.value("appid", 0);
        g.name = j.value("name", "");
        g.is_steam_app = j.value("is_steam_app", true);
        g.launch_opts = j.value("launch_opts", "%command%");

        return g;
    }
};

inline void from_json(const json &j, SteamGameDetails &g)
{
    g.appid = j.value("appid", 0);
    g.name = j.value("name", "");
    g.is_steam_app = j.value("is_steam_app", true);
    g.launch_opts = j.value("launch_opts", "%command%");
}

inline void to_json(json &j, const SteamGameDetails &g)
{
    j = json{
        {"appid", g.appid},
        {"name", g.name},
        {"is_steam_app", g.is_steam_app},
        {"launch_opts", g.launch_opts}};
}