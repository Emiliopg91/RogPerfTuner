#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "../../utils/constants.hpp"
using json = nlohmann::json;

struct Application {
	std::string latestVersion = Constants::APP_VERSION;
};

inline void to_json(nlohmann::json& j, const Application& o) {
	j				   = json{};
	j["latestVersion"] = o.latestVersion;
}
inline void from_json(const nlohmann::json& j, Application& o) {
	if (j.contains("latestVersion")) {
		j.at("latestVersion").get_to(o.latestVersion);
	}
}