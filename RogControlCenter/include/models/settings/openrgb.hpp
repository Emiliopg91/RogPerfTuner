#pragma once

#include <map>
#include <string>

#include "../hardware/rgb_brightness.hpp"
#include "effect.hpp"

struct OpenRGB {
	RgbBrightness brightness							  = RgbBrightness::Enum::MAX;
	std::unordered_map<std::string, EffectConfig> effects = {};
	std::optional<std::string> last_effect				  = std::nullopt;
};

inline void to_json(nlohmann::json& j, const OpenRGB& o) {
	j				 = json{};
	j["brightness"]	 = o.brightness.toInt();
	j["effects"]	 = o.effects;
	j["last_effect"] = o.last_effect ? json(*o.last_effect) : json(nullptr);
}
inline void from_json(const nlohmann::json& j, OpenRGB& o) {
	o.brightness = RgbBrightness::fromInt(j.at("brightness").get<int>());
	j.at("effects").get_to(o.effects);

	if (j.contains("last_effect") && !j.at("last_effect").is_null()) {
		o.last_effect = j.at("last_effect").get<std::string>();
	} else {
		o.last_effect = std::nullopt;
	}
}