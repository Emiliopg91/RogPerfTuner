#pragma once

#include <yaml-cpp/yaml.h>

struct Application {
	bool askedInstallRccdc = false;
	bool enroled		   = false;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Application> {
	static Node encode(const Application& app) {
		Node node;
		node["askedInstallRccdc"] = app.askedInstallRccdc;
		node["enroled"]			  = app.enroled;
		return node;
	}

	static bool decode(const Node& node, Application& app) {
		if (node["askedInstallRccdc"]) {
			app.askedInstallRccdc = node["askedInstallRccdc"].as<bool>();
		}
		if (node["enroled"]) {
			app.enroled = node["enroled"].as<bool>();
		}
		return true;
	}
};
}  // namespace YAML