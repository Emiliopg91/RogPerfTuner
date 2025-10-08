#pragma once

#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

struct EffectConfig {
	std::string color;
};

inline void to_json(nlohmann::json& j, const EffectConfig& e) {
	j = nlohmann::json{{"color", e.color}};
}
inline void from_json(const nlohmann::json& j, EffectConfig& e) {
	j.at("color").get_to(e.color);
}