#pragma once

#include "platform_profile.hpp"

struct PlatformConfig
{
    PlatformProfiles profiles = PlatformProfiles();
};

inline void to_json(nlohmann::json &j, const PlatformConfig &e)
{
    j = nlohmann::json{{"profiles", e.profiles}};
}
inline void from_json(const nlohmann::json &j, PlatformConfig &e)
{
    j.at("profiles").get_to(e.profiles);
}