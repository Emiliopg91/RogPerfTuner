#pragma once

#include <string>
#include <map>
#include <optional>
#include <cstdint>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "mangohud_level.hpp"
#include "wine_sync_option.hpp"
#include "performance_profile.hpp"
#include "rgb_brightness.hpp"

// --- games section ---
struct GameEntry
{
    std::optional<std::string> args = std::nullopt;
    std::optional<std::string> env = std::nullopt;
    std::optional<std::string> gpu = std::nullopt;
    MangoHudLevel metrics_level = MangoHudLevel::Enum::NO_DISPLAY;
    std::string name;
    std::string overlayId;
    bool proton = true;
    bool steamdeck = false;
    WineSyncOption sync = WineSyncOption::Enum::AUTO;
};

// --- Effect config ---
struct EffectConfig
{
    std::string color;
};

// --- OpenRGB config --- //
struct OpenRGB
{
    RgbBrightness brightness = RgbBrightness::Enum::MAX;
    std::map<std::string, EffectConfig> effects = {};
    std::optional<std::string> last_effect = std::nullopt;
};
// --- Platform profiles --- //
struct PlatformProfiles
{
    PerformanceProfile profile = PerformanceProfile::Enum::QUIET;
};
// --- Platform config ---//
struct PlatformConfig
{
    PlatformProfiles profiles = PlatformProfiles();
};

// --- root config ---
struct RootConfig
{
    std::map<std::string, GameEntry> games = {};
    std::map<std::string, std::string> logger = {};
    OpenRGB open_rgb = OpenRGB();
    PlatformConfig platform = PlatformConfig();
};

inline void to_json(json &j, const GameEntry &g)
{
    j = json{};

    j["args"] = g.args ? json(*g.args) : json(nullptr);
    j["env"] = g.env ? json(*g.env) : json(nullptr);
    j["gpu"] = g.gpu ? json(*g.gpu) : json(nullptr);

    j["metrics_level"] = g.metrics_level.toInt();
    j["name"] = g.name;
    j["overlayId"] = g.overlayId;
    j["proton"] = g.proton;
    j["steamdeck"] = g.steamdeck;
    j["sync"] = g.sync.toString();
}

inline void from_json(const json &j, GameEntry &g)
{
    // Manejo de opcionales
    if (j.contains("args") && !j.at("args").is_null())
        g.args = j.at("args").get<std::string>();
    else
        g.args = std::nullopt;

    if (j.contains("env") && !j.at("env").is_null())
        g.env = j.at("env").get<std::string>();
    else
        g.env = std::nullopt;

    if (j.contains("gpu") && !j.at("gpu").is_null())
        g.gpu = j.at("gpu").get<std::string>();
    else
        g.gpu = std::nullopt;

    // Campos obligatorios (sin optional)
    g.metrics_level = MangoHudLevel::fromInt(j.at("metrics_level").get<int>());
    g.name = j.at("name").get<std::string>();
    g.overlayId = j.at("overlayId").get<std::string>();
    g.proton = j.at("proton").get<bool>();
    g.steamdeck = j.at("steamdeck").get<bool>();
    g.sync = WineSyncOption::fromString(j.at("sync").get<std::string>());
}

inline void to_json(nlohmann::json &j, const EffectConfig &e)
{
    j = nlohmann::json{{"color", e.color}};
}
inline void from_json(const nlohmann::json &j, EffectConfig &e)
{
    j.at("color").get_to(e.color);
}

inline void to_json(nlohmann::json &j, const OpenRGB &o)
{
    j = json{};
    j["brightness"] = o.brightness.toInt();
    j["effects"] = o.effects;
    j["last_effect"] = o.last_effect ? json(*o.last_effect) : json(nullptr);
}
inline void from_json(const nlohmann::json &j, OpenRGB &o)
{
    o.brightness = RgbBrightness::fromInt(j.at("brightness").get<int>());
    j.at("effects").get_to(o.effects);

    if (j.contains("last_effect") && !j.at("last_effect").is_null())
        o.last_effect = j.at("last_effect").get<std::string>();
    else
        o.last_effect = std::nullopt;
}

inline void to_json(nlohmann::json &j, const PlatformProfiles &o)
{
    j = nlohmann::json{
        {"profile", o.profile.toInt()}};
}
inline void from_json(const nlohmann::json &j, PlatformProfiles &o)
{
    o.profile = PerformanceProfile::fromInt(j.at("profile").get<int>());
}

inline void to_json(nlohmann::json &j, const PlatformConfig &e)
{
    j = nlohmann::json{{"profiles", e.profiles}};
}
inline void from_json(const nlohmann::json &j, PlatformConfig &e)
{
    j.at("profiles").get_to(e.profiles);
}

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