#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "game_entry.hpp"
#include "openrgb.hpp"
#include "performance.hpp"

struct RootConfig {
	std::unordered_map<std::string, GameEntry> games	= {};
	std::unordered_map<std::string, std::string> logger = {{"Default", "INFO"}};
	OpenRGB open_rgb									= OpenRGB();
	Performance performance								= Performance();
};

inline void to_json(nlohmann::json& j, const RootConfig& r) {
	j = nlohmann::json{{"games", r.games}, {"logger", r.logger}, {"open_rgb", r.open_rgb}, {"performance", r.performance}};
}

inline void from_json(const nlohmann::json& j, RootConfig& r) {
	// games: si no está, queda vacío
	r.games = j.contains("games") ? j.at("games").get<std::unordered_map<std::string, GameEntry>>() : std::unordered_map<std::string, GameEntry>{};

	// logger: si no está, queda vacío
	r.logger =
		j.contains("logger") ? j.at("logger").get<std::unordered_map<std::string, std::string>>() : std::unordered_map<std::string, std::string>{};

	// open_rgb: si no está, queda en default
	r.open_rgb = j.contains("open_rgb") ? j.at("open_rgb").get<OpenRGB>() : OpenRGB{};

	// performance: si no está, queda en default
	r.performance = j.contains("performance") ? j.at("performance").get<Performance>() : Performance{};
}