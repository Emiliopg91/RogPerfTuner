#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../performance/performance_profile.hpp"

struct PlatformProfiles {
	PerformanceProfile profile = PerformanceProfile::Enum::QUIET;
};

inline void to_json(nlohmann::json& j, const PlatformProfiles& o) {
	j = nlohmann::json{{"profile", o.profile.toString()}};
}
inline void from_json(const nlohmann::json& j, PlatformProfiles& o) {
	o.profile = PerformanceProfile::fromString(j.at("profile").get<std::string>());
}