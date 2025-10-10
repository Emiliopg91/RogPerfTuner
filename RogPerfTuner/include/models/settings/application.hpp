#pragma once

#include <yaml-cpp/yaml.h>

#include "../../utils/constants.hpp"

struct Application {
	bool askedInstallRccdc = false;
	bool appimage		   = Constants::APPIMAGE_FILE.has_value();
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Application> {
	static Node encode(const Application& app) {
		Node node;
		node["appimage"]		  = app.appimage;
		node["askedInstallRccdc"] = app.askedInstallRccdc;
		return node;
	}

	static bool decode(const Node& node, Application& app) {
		if (node["askedInstallRccdc"]) {
			app.askedInstallRccdc = node["askedInstallRccdc"].as<bool>();
		}
		if (node["appimage"]) {
			app.appimage = node["appimage"].as<bool>();
		}
		return true;
	}
};
}  // namespace YAML