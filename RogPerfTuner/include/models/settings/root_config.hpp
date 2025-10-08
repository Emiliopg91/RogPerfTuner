#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

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

inline void to_json(nlohmann::json& j, const RootConfig& r) {
	j = nlohmann::json{};
	if (!r.games.empty()) {
		j["games"] = r.games;
	}
	if (!r.logger.empty()) {
		j["logger"] = r.logger;
	}

	j["application"] = r.application;
	j["aura"]		 = r.aura;
	j["platform"]	 = r.platform;
}

inline void from_json(const nlohmann::json& j, RootConfig& r) {
	// games: si no está, queda vacío
	r.games = j.contains("games") ? j.at("games").get<std::unordered_map<std::string, GameEntry>>() : std::unordered_map<std::string, GameEntry>{};

	// logger: si no está, queda vacío
	r.logger =
		j.contains("logger") ? j.at("logger").get<std::unordered_map<std::string, std::string>>() : std::unordered_map<std::string, std::string>{};

	// aura: si no está, queda en default
	r.aura = j.contains("aura") ? j.at("aura").get<Aura>() : Aura{};

	// performance: si no está, queda en default
	r.platform = j.contains("platform") ? j.at("platform").get<Platform>() : Platform{};

	// performance: si no está, queda en default
	r.application = j.contains("application") ? j.at("application").get<Application>() : Application{};
}