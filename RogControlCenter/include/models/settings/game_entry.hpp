#pragma once

#include <nlohmann/json.hpp>
#include <optional>
using json = nlohmann::json;

#include "../steam/mangohud_level.hpp"
#include "../steam/wine_sync_option.hpp"

struct GameEntry {
	std::optional<std::string> args = std::nullopt;
	std::optional<std::string> env	= std::nullopt;
	std::optional<std::string> gpu	= std::nullopt;
	MangoHudLevel metrics_level		= MangoHudLevel::Enum::NO_DISPLAY;
	std::string name;
	std::string overlayId;
	bool proton			= true;
	bool steamdeck		= false;
	WineSyncOption sync = WineSyncOption::Enum::AUTO;
};

inline void to_json(json& j, const GameEntry& g) {
	j = json{};

	j["args"] = g.args ? json(*g.args) : json(nullptr);
	j["env"]  = g.env ? json(*g.env) : json(nullptr);
	j["gpu"]  = g.gpu ? json(*g.gpu) : json(nullptr);

	j["metrics_level"] = g.metrics_level.toInt();
	j["name"]		   = g.name;
	j["overlayId"]	   = g.overlayId;
	j["proton"]		   = g.proton;
	j["steamdeck"]	   = g.steamdeck;
	j["sync"]		   = g.sync.toString();
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

	if (j.contains("gpu") && !j.at("gpu").is_null()) {
		g.gpu = j.at("gpu").get<std::string>();
	} else {
		g.gpu = std::nullopt;
	}

	// Campos obligatorios (sin optional)
	g.metrics_level = MangoHudLevel::fromInt(j.at("metrics_level").get<int>());
	g.name			= j.at("name").get<std::string>();
	g.overlayId		= j.at("overlayId").get<std::string>();
	g.proton		= j.at("proton").get<bool>();
	g.steamdeck		= j.at("steamdeck").get<bool>();
	g.sync			= WineSyncOption::fromString(j.at("sync").get<std::string>());
}