#pragma once

#include <yaml-cpp/yaml.h>

#include <string>

#include "enum_utils.hpp"
#include "models/hardware/rgb_brightness.hpp"
#include "models/settings/effect.hpp"

struct Aura {
	RgbBrightness brightness				   = RgbBrightness::MAX;
	std::map<std::string, EffectConfig> config = {};
	std::optional<std::string> last_effect	   = std::nullopt;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Aura> {
	static Node encode(const Aura& aura) {
		Node node;
		node["brightness"] = toInt(aura.brightness);
		if (!aura.config.empty()) {
			node["config"] = aura.config;
		}
		if (aura.last_effect) {
			node["effect"] = *aura.last_effect;
		}
		return node;
	}

	static bool decode(const Node& node, Aura& aura) {
		if (node["brightness"]) {
			aura.brightness = fromInt<RgbBrightness>(node["brightness"].as<int>());
		}

		if (node["config"]) {
			aura.config = node["config"].as<std::map<std::string, EffectConfig>>();
		}

		if (node["effect"]) {
			aura.last_effect = node["effect"].as<std::string>();
		}

		return true;
	}
};
}  // namespace YAML