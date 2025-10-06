#pragma once
#include <nlohmann/json.hpp>

#include "../../utils/constants.hpp"
using json = nlohmann::json;

struct Application {
	bool askedInstallRccdc = false;
	bool appimage		   = Constants::APPIMAGE_FILE.has_value();
};

inline void to_json(nlohmann::json& j, const Application& o) {
	j					   = json{};
	j["askedInstallRccdc"] = o.askedInstallRccdc;
	j["appimage"]		   = o.appimage;
}
inline void from_json(const nlohmann::json& j, Application& o) {
	if (j.contains("askedInstallRccdc")) {
		j.at("askedInstallRccdc").get_to(o.askedInstallRccdc);
	}
	if (j.contains("appimage")) {
		j.at("appimage").get_to(o.appimage);
	}
}