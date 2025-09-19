#pragma once

#include <string>

#include "../hardware/rgb_brightness.hpp"
#include "effect.hpp"

struct Aura {
	RgbBrightness brightness							 = RgbBrightness::Enum::MAX;
	std::unordered_map<std::string, EffectConfig> config = {};
	std::optional<std::string> last_effect				 = std::nullopt;
};

inline void to_json(nlohmann::json& j, const Aura& o) {
	j				= json{};
	j["brightness"] = o.brightness.toInt();
	if (!o.config.empty()) {
		j["config"] = o.config;
	}
	j["effect"] = o.last_effect ? json(*o.last_effect) : json(nullptr);
}
inline void from_json(const nlohmann::json& j, Aura& o) {
	o.brightness = RgbBrightness::fromInt(j.at("brightness").get<int>());
	if (j.contains("config")) {
		j.at("config").get_to(o.config);
	} else if (j.contains("effects")) {
		j.at("effects").get_to(o.config);
	}

	if (j.contains("effect") && !j.at("effect").is_null()) {
		o.last_effect = j.at("effect").get<std::string>();
	} else if (j.contains("last_effect") && !j.at("last_effect").is_null()) {
		o.last_effect = j.at("last_effect").get<std::string>();
	} else {
		o.last_effect = std::nullopt;
	}
}