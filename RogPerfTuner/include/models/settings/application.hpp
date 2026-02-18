#pragma once

#include <yaml-cpp/yaml.h>

#include "utils/constants.hpp"

struct Application {
	bool askedInstallRccdc = false;
	bool startMinimized	   = true;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Application> {
	static Node encode(const Application& app) {
		Node node;
		node["askedInstallRccdc"] = app.askedInstallRccdc;
		node["startMinimized"]	  = app.startMinimized;
		return node;
	}

	static bool decode(const Node& node, Application& app) {
		if (node["askedInstallRccdc"]) {
			app.askedInstallRccdc = node["askedInstallRccdc"].as<bool>();
		}
		if (node["startMinimized"]) {
			app.startMinimized = node["startMinimized"].as<bool>();
		}
		return true;
	}
};
}  // namespace YAML