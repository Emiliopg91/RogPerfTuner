#pragma once

#include <nlohmann/json.hpp>
#include <optional>
using json = nlohmann::json;

#include "../performance/performance_profile.hpp"

struct Performance {
	PerformanceProfile profile			 = PerformanceProfile::Enum::QUIET;
	std::optional<std::string> scheduler = std::nullopt;
};

inline void to_json(nlohmann::json& j, const Performance& o) {
	j			 = nlohmann::json{};
	j["profile"] = o.profile.toString();
	if (o.scheduler.has_value()) {
		j["scheduler"] = o.scheduler.value();
	}
}
inline void from_json(const nlohmann::json& j, Performance& o) {
	o.profile = PerformanceProfile::fromString(j.at("profile").get<std::string>());
	if (j.contains("scheduler")) {
		o.scheduler = j["scheduler"];
	}
}