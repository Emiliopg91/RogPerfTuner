#pragma once

#include <yaml-cpp/yaml.h>

#include "models/logger_level.hpp"
#include "models/settings/application.hpp"
#include "models/settings/aura.hpp"
#include "models/settings/game_entry.hpp"
#include "models/settings/platform.hpp"
#include "utils/enum_utils.hpp"

struct RootConfig {
	Aura aura								  = Aura();
	Application application					  = Application();
	std::map<uint, GameEntry> games			  = {};
	std::map<std::string, LoggerLevel> logger = {};
	Platform platform						  = Platform();
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<RootConfig> {
	static Node encode(const RootConfig& config) {
		Node node;

		node["application"] = config.application;
		node["aura"]		= config.aura;

		node["games"] = config.games;

		Node loggerNode(NodeType::Map);
		if (!config.logger.empty()) {
			for (const auto& [key, level] : config.logger) {
				loggerNode[key] = toName(level);
			}
		}
		node["logger"] = loggerNode;

		node["platform"] = config.platform;

		return node;
	}

	static bool decode(const Node& node, RootConfig& config) {
		if (node["games"]) {
			config.games = node["games"].as<std::map<uint, GameEntry>>();
		} else {
			config.games = std::map<uint, GameEntry>{};
		}

		if (node["logger"]) {
			auto loggerNode = node["logger"];
			config.logger.clear();
			for (const auto& it : loggerNode) {
				config.logger[it.first.as<std::string>()] = fromString<LoggerLevel>(it.second.as<std::string>());
			}
		} else {
			config.logger.clear();
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