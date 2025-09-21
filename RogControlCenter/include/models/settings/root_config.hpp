#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "aura.hpp"
#include "game_entry.hpp"
#include "platform.hpp"

struct RootConfig {
	std::unordered_map<std::string, GameEntry> games	= {};
	std::unordered_map<std::string, std::string> logger = {};
	Aura aura											= Aura();
	Platform platform									= Platform();
};

inline void to_json(nlohmann::json& j, const RootConfig& r) {
	j = nlohmann::json{};
	if (!r.games.empty()) {
		j["games"] = r.games;
	}
	if (!r.logger.empty()) {
		j["logger"] = r.logger;
	}
	j["aura"]	  = r.aura;
	j["platform"] = r.platform;
}

inline void from_json(const nlohmann::json& j, RootConfig& r) {
	r.games = j.contains("games") ? j.at("games").get<std::unordered_map<std::string, GameEntry>>() : std::unordered_map<std::string, GameEntry>{};

	r.logger =
		j.contains("logger") ? j.at("logger").get<std::unordered_map<std::string, std::string>>() : std::unordered_map<std::string, std::string>{};

	r.aura = j.contains("aura") ? j.at("aura").get<Aura>() : Aura{};

	r.platform = j.contains("platform") ? j.at("platform").get<Platform>() : Platform{};
}