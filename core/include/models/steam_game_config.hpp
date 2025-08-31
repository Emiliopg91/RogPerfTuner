#pragma once

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct SteamGameConfig
{
    std::map<std::string, std::string> environment;
    std::vector<std::string> wrappers;
    std::string parameters;
};

inline void to_json(json &j, const SteamGameConfig &d)
{
    j = json{
        {"environment", d.environment},
        {"wrappers", d.wrappers},
        {"parameters", d.parameters}};
}

inline void from_json(const json &j, SteamGameConfig &d)
{
    j.at("environment").get_to(d.environment);
    j.at("wrappers").get_to(d.wrappers);
    j.at("parameters").get_to(d.parameters);
}