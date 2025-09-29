#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Application {};

inline void to_json(nlohmann::json& j, const Application&) {
	j = json{};
	// j["latestVersion"] = o.latestVersion;
}
inline void from_json(const nlohmann::json&, Application&) {
	/*if (j.contains("latestVersion")) {
		j.at("latestVersion").get_to(o.latestVersion);
	}*/
}