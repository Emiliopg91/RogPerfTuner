#pragma once

#include <yaml-cpp/yaml.h>

#include "../../utils/constants.hpp"

struct Application {
	bool askedInstallRccdc		= false;
	bool enrolled				= false;
	bool startMinimized			= true;
	std::string currentVersion	= Constants::APP_VERSION;
	std::string previousVersion = Constants::APP_VERSION;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Application> {
	static Node encode(const Application& app) {
		Node node;
		node["enrolled"]		  = app.enrolled;
		node["askedInstallRccdc"] = app.askedInstallRccdc;
		node["startMinimized"]	  = app.startMinimized;
		node["currentVersion"]	  = app.currentVersion;
		node["previousVersion"]	  = app.previousVersion;
		return node;
	}

	static bool decode(const Node& node, Application& app) {
		if (node["askedInstallRccdc"]) {
			app.askedInstallRccdc = node["askedInstallRccdc"].as<bool>();
		}
		if (node["enrolled"]) {
			app.enrolled = node["enrolled"].as<bool>();
		}
		if (node["startMinimized"]) {
			app.enrolled = node["startMinimized"].as<bool>();
		}
		if (node["currentVersion"]) {
			app.currentVersion = node["currentVersion"].as<std::string>();
		}
		if (node["previousVersion"]) {
			app.previousVersion = node["previousVersion"].as<std::string>();
		}
		return true;
	}
};
}  // namespace YAML