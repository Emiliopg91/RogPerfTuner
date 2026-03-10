/**
 * @file steam_game_config.hpp
 * @author Emili...
 * @brief YAML-CPP conversion helpers for SteamGameConfig
 * @version 4.0.0
 * @date 2025-09-10
 */

#pragma once
#include <yaml-cpp/yaml.h>

#include <string>
#include <unordered_map>
#include <vector>

struct SteamGameConfig {
	std::unordered_map<std::string, std::string> environment;
	std::vector<std::string> wrappers;
	std::string parameters;
};

namespace YAML {
template <>
struct convert<SteamGameConfig> {
	static Node encode(const SteamGameConfig& d) {
		Node node;
		node["environment"] = d.environment;
		node["wrappers"]	= d.wrappers;
		node["parameters"]	= d.parameters;
		return node;
	}

	static bool decode(const Node& node, SteamGameConfig& d) {
		if (!node.IsMap()) {
			return false;
		}

		if (node["environment"]) {
			d.environment = node["environment"].as<std::unordered_map<std::string, std::string>>();
		}

		if (node["wrappers"]) {
			d.wrappers = node["wrappers"].as<std::vector<std::string>>();
		}

		if (node["parameters"]) {
			d.parameters = node["parameters"].as<std::string>();
		}

		return true;
	}
};
}  // namespace YAML
