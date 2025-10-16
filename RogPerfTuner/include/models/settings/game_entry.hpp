#pragma once

#include <yaml-cpp/yaml.h>

#include <optional>

#include "../steam/mangohud_level.hpp"
#include "../steam/wine_sync_option.hpp"

struct GameEntry {
	std::optional<std::string> args		 = std::nullopt;
	std::optional<std::string> env		 = std::nullopt;
	std::optional<std::string> gpu		 = std::nullopt;
	std::optional<std::string> scheduler = std::nullopt;
	MangoHudLevel metrics_level			 = MangoHudLevel::Enum::NO_DISPLAY;
	std::string name;
	std::optional<std::string> overlayId;
	bool proton							= true;
	bool steamdeck						= false;
	WineSyncOption sync					= WineSyncOption::Enum::AUTO;
	std::optional<std::string> wrappers = std::nullopt;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<GameEntry> {
	static Node encode(const GameEntry& game) {
		Node node;

		node["name"] = game.name;

		if (game.args && !game.args->empty()) {
			node["args"] = *game.args;
		}
		if (game.env && !game.env->empty()) {
			node["env"] = *game.env;
		}
		if (game.gpu && !game.gpu->empty()) {
			node["gpu"] = *game.gpu;
		}
		node["metrics"] = game.metrics_level.toString();
		if (game.overlayId && !game.overlayId->empty()) {
			node["overlayId"] = *game.overlayId;
		}
		if (!game.proton) {
			node["proton"] = false;
		}
		if (game.scheduler && !game.scheduler->empty()) {
			node["scheduler"] = *game.scheduler;
		}
		if (game.proton) {
			if (game.steamdeck) {
				node["steamdeck"] = true;
			}
			node["sync"] = game.sync.toString();
		}
		if (game.wrappers && !game.wrappers->empty()) {
			node["wrappers"] = *game.wrappers;
		}

		return node;
	}

	static bool decode(const Node& node, GameEntry& game) {
		if (node["args"] && !node["args"].IsNull()) {
			game.args = node["args"].as<std::string>();
		} else {
			game.args = std::nullopt;
		}

		if (node["env"] && !node["env"].IsNull()) {
			game.env = node["env"].as<std::string>();
		} else {
			game.env = std::nullopt;
		}

		if (node["wrappers"] && !node["wrappers"].IsNull()) {
			game.wrappers = node["wrappers"].as<std::string>();
		} else {
			game.wrappers = std::nullopt;
		}

		if (node["overlayId"] && !node["overlayId"].IsNull()) {
			game.overlayId = node["overlayId"].as<std::string>();
		} else {
			game.overlayId = std::nullopt;
		}

		if (node["gpu"] && !node["gpu"].IsNull()) {
			game.gpu = node["gpu"].as<std::string>();
		} else {
			game.gpu = std::nullopt;
		}

		if (node["scheduler"] && !node["scheduler"].IsNull()) {
			game.scheduler = node["scheduler"].as<std::string>();
		} else {
			game.scheduler = std::nullopt;
		}

		if (node["proton"]) {
			game.proton = node["proton"].as<bool>();
		} else {
			game.proton = true;
		}

		if (node["steamdeck"]) {
			game.steamdeck = node["steamdeck"].as<bool>();
		} else {
			game.steamdeck = false;
		}

		if (node["sync"]) {
			game.sync = WineSyncOption::fromString(node["sync"].as<std::string>());
		} else {
			game.sync = WineSyncOption::Enum::AUTO;
		}

		if (node["metrics"]) {
			game.metrics_level = MangoHudLevel::fromString(node["metrics"].as<std::string>());
		}
		if (node["name"]) {
			game.name = node["name"].as<std::string>();
		}

		return true;
	}
};
}  // namespace YAML