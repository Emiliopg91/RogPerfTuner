#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct SteamGameConfig {
	std::map<std::string, std::string> environment;
	std::vector<std::string> wrappers;
	std::string parameters;
};

inline void to_json(nlohmann::json& j, const SteamGameConfig& d) {
	j = nlohmann::json{{"environment", d.environment}, {"wrappers", d.wrappers}, {"parameters", d.parameters}};
}

inline void from_json(const nlohmann::json& j, SteamGameConfig& d) {
	j.at("environment").get_to(d.environment);
	j.at("wrappers").get_to(d.wrappers);
	j.at("parameters").get_to(d.parameters);
}
