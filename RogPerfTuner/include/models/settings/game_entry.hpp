#pragma once

#include <yaml-cpp/yaml.h>

#include <optional>

#include "models/steam/computer_type.hpp"
#include "models/steam/mangohud_level.hpp"
#include "models/steam/wine_sync_option.hpp"
#include "utils/enum_utils.hpp"

struct GameEntry {
	inline static const MangoHudLevel DEFAULT_METRICS_LEVEL = MangoHudLevel::NO_DISPLAY;
	inline static const ComputerType DEFAULT_DEVICE			= ComputerType::COMPUTER;
	inline static const WineSyncOption DEFAULT_SYNC			= WineSyncOption::AUTO;
	inline static const bool DEFAULT_PROTON					= true;

	std::optional<std::string> args		 = std::nullopt;
	std::optional<std::string> env		 = std::nullopt;
	std::optional<std::string> gpu		 = std::nullopt;
	std::optional<std::string> scheduler = std::nullopt;
	MangoHudLevel metrics_level			 = DEFAULT_METRICS_LEVEL;
	std::string name;
	std::optional<std::string> overlayId;
	bool proton							= DEFAULT_PROTON;
	ComputerType device					= DEFAULT_DEVICE;
	WineSyncOption sync					= DEFAULT_SYNC;
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
		if (game.metrics_level != GameEntry::DEFAULT_METRICS_LEVEL) {
			node["metrics"] = toString(game.metrics_level);
		}
		if (game.overlayId && !game.overlayId->empty()) {
			node["overlayId"] = *game.overlayId;
		}
		if (game.proton != GameEntry::DEFAULT_PROTON) {
			node["proton"] = game.proton;
		}
		if (game.proton) {
			if (game.device != ComputerType::COMPUTER) {
				node["device"] = toString(game.device);
			}
			if (game.sync != WineSyncOption::AUTO) {
				node["sync"] = toString(game.sync);
			}
		}
		if (game.scheduler && !game.scheduler->empty()) {
			node["scheduler"] = *game.scheduler;
		}
		if (game.wrappers && !game.wrappers->empty()) {
			node["wrappers"] = *game.wrappers;
		}

		return node;
	}

	static bool decode(const Node& node, GameEntry& game) {
		if (node["args"] && !node["args"].IsNull()) {
			game.args = node["args"].as<std::string>();
		}

		if (node["env"] && !node["env"].IsNull()) {
			game.env = node["env"].as<std::string>();
		}

		if (node["wrappers"] && !node["wrappers"].IsNull()) {
			game.wrappers = node["wrappers"].as<std::string>();
		}

		if (node["overlayId"] && !node["overlayId"].IsNull()) {
			game.overlayId = node["overlayId"].as<std::string>();
		}

		if (node["gpu"] && !node["gpu"].IsNull()) {
			game.gpu = node["gpu"].as<std::string>();
		}

		if (node["scheduler"] && !node["scheduler"].IsNull()) {
			game.scheduler = node["scheduler"].as<std::string>();
		}

		if (node["proton"]) {
			game.proton = node["proton"].as<bool>();
		}

		if (node["computer"]) {
			game.device = fromString<ComputerType>(node["computer"].as<std::string>());
		} else {
			if (node["steamdeck"]) {
				game.device = node["steamdeck"].as<bool>() ? ComputerType::STEAM_DECK : GameEntry::DEFAULT_DEVICE;
			} else {
				if (node["device"]) {
					game.device = fromString<ComputerType>(node["device"].as<std::string>());
				}
			}
		}

		if (node["sync"]) {
			game.sync = fromString<WineSyncOption>(node["sync"].as<std::string>());
		}

		if (node["metrics"]) {
			game.metrics_level = fromString<MangoHudLevel>(node["metrics"].as<std::string>());
		}

		if (node["name"]) {
			game.name = node["name"].as<std::string>();
		}

		return true;
	}
};
}  // namespace YAML