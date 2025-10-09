#pragma once

#include <yaml-cpp/yaml.h>

#include "application.hpp"
#include "aura.hpp"
#include "game_entry.hpp"
#include "platform.hpp"

struct RootConfig {
	Aura aura											= Aura();
	Application application								= Application();
	std::unordered_map<std::string, GameEntry> games	= {};
	std::unordered_map<std::string, std::string> logger = {};
	Platform platform									= Platform();
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<RootConfig> {
	static Node encode(const RootConfig& config) {
		Node node;

		node["application"] = config.application;
		node["aura"]		= config.aura;

		if (!config.games.empty()) {
			node["games"] = config.games;
		}
		if (!config.logger.empty()) {
			node["logger"] = config.logger;
		}

		node["platform"] = config.platform;

		return node;
	}

	static bool decode(const Node& node, RootConfig& config) {
		if (node["games"]) {
			config.games = node["games"].as<std::unordered_map<std::string, GameEntry>>();
		} else {
			config.games = std::unordered_map<std::string, GameEntry>{};
		}

		if (node["logger"]) {
			config.logger = node["logger"].as<std::unordered_map<std::string, std::string>>();
		} else {
			config.logger = std::unordered_map<std::string, std::string>{};
		}

		if (node["aura"]) {
			config.aura = node["aura"].as<Aura>();
		} else {
			config.aura = Aura{};
		}

		if (node["platform"]) {
			config.platform = node["platform"].as<Platform>();
		} else {
			config.platform = Platform{};
		}

		if (node["application"]) {
			config.application = node["application"].as<Application>();
		} else {
			config.application = Application{};
		}

		return true;
	}
};
}  // namespace YAML