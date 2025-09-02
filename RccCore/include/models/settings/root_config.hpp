#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "game_entry.hpp"
#include "openrgb.hpp"
#include "platform_config.hpp"

struct RootConfig
{
    std::map<std::string, GameEntry> games = {};
    std::map<std::string, std::string> logger = {};
    OpenRGB open_rgb = OpenRGB();
    PlatformConfig platform = PlatformConfig();
};

inline void to_json(nlohmann::json &j, const RootConfig &r)
{
    j = nlohmann::json{
        {"games", r.games},
        {"logger", r.logger},
        {"open_rgb", r.open_rgb},
        {"platform", r.platform}};
}

inline void from_json(const nlohmann::json &j, RootConfig &r)
{
    // games: si no está, queda vacío
    r.games = j.contains("games")
                  ? j.at("games").get<std::map<std::string, GameEntry>>()
                  : std::map<std::string, GameEntry>{};

    // logger: si no está, queda vacío
    r.logger = j.contains("logger")
                   ? j.at("logger").get<std::map<std::string, std::string>>()
                   : std::map<std::string, std::string>{};

    // open_rgb: si no está, queda en default
    r.open_rgb = j.contains("open_rgb")
                     ? j.at("open_rgb").get<OpenRGB>()
                     : OpenRGB{};

    // platform: si no está, queda en default
    r.platform = j.contains("platform")
                     ? j.at("platform").get<PlatformConfig>()
                     : PlatformConfig{};
}