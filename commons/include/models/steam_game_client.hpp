#pragma once
#include <string>
struct SteamGameDetails
{
    int appid;
    std::string name;
    bool is_steam_app = true;
    std::string launch_opts = "%command%";
};