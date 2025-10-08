#pragma once

#include <nlohmann/json.hpp>
#include <optional>
using json = nlohmann::json;

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

inline void to_json(json& j, const GameEntry& g) {
	j = json{};

	if (!g.args.value_or("").empty()) {
		j["args"] = json(*g.args);
	}
	if (!g.env.value_or("").empty()) {
		j["env"] = json(*g.env);
	}
	if (!g.wrappers.value_or("").empty()) {
		j["wrappers"] = json(*g.wrappers);
	}
	if (!g.gpu.value_or("").empty()) {
		j["gpu"] = json(*g.gpu);
	}
	if (!g.overlayId.value_or("").empty()) {
		j["overlayId"] = json(*g.overlayId);
	}
	if (!g.scheduler.value_or("").empty()) {
		j["scheduler"] = json(*g.scheduler);
	}
	if (!g.proton) {
		j["proton"] = false;
	}
	if (g.proton) {
		if (g.steamdeck) {
			j["steamdeck"] = true;
		}
		j["sync"] = g.sync.toString();
	}

	j["metrics"] = g.metrics_level.toString();
	j["name"]	 = g.name;
}

inline void from_json(const json& j, GameEntry& g) {
	// Manejo de opcionales
	if (j.contains("args") && !j.at("args").is_null()) {
		g.args = j.at("args").get<std::string>();
	} else {
		g.args = std::nullopt;
	}

	if (j.contains("env") && !j.at("env").is_null()) {
		g.env = j.at("env").get<std::string>();
	} else {
		g.env = std::nullopt;
	}

	if (j.contains("wrappers") && !j.at("wrappers").is_null()) {
		g.wrappers = j.at("wrappers").get<std::string>();
	} else {
		g.wrappers = std::nullopt;
	}

	if (j.contains("overlayId") && !j.at("overlayId").is_null()) {
		g.overlayId = j.at("overlayId").get<std::string>();
	} else {
		g.overlayId = std::nullopt;
	}

	if (j.contains("gpu") && !j.at("gpu").is_null()) {
		g.gpu = j.at("gpu").get<std::string>();
	} else {
		g.gpu = std::nullopt;
	}

	if (j.contains("scheduler") && !j.at("scheduler").is_null()) {
		g.scheduler = j.at("scheduler").get<std::string>();
	} else {
		g.scheduler = std::nullopt;
	}

	if (j.contains("proton")) {
		g.proton = j.at("proton").get<bool>();
	} else {
		g.proton = true;
	}

	if (j.contains("steamdeck")) {
		g.steamdeck = j.at("steamdeck").get<bool>();
	} else {
		g.steamdeck = false;
	}

	if (j.contains("sync")) {
		g.sync = j.at("sync").get<std::string>();
	} else {
		g.sync = ((WineSyncOption)WineSyncOption::Enum::AUTO).toString();
	}

	g.metrics_level = MangoHudLevel::fromString(j.at("metrics").get<std::string>());
	g.name			= j.at("name").get<std::string>();
}