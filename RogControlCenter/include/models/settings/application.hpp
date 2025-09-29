#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Application {
	bool askedInstallRccdc = false;
};

inline void to_json(nlohmann::json& j, const Application& o) {
	j					   = json{};
	j["askedInstallRccdc"] = o.askedInstallRccdc;
}
inline void from_json(const nlohmann::json& j, Application& o) {
	if (j.contains("askedInstallRccdc")) {
		j.at("askedInstallRccdc").get_to(o.askedInstallRccdc);
	}
}