#pragma once

#include <yaml-cpp/yaml.h>

#include <string>

#include "../hardware/rgb_brightness.hpp"
#include "effect.hpp"

struct Aura {
	RgbBrightness brightness							 = RgbBrightness::Enum::MAX;
	std::unordered_map<std::string, EffectConfig> config = {};
	std::optional<std::string> last_effect				 = std::nullopt;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Aura> {
	static Node encode(const Aura& aura) {
		Node node;
		node["brightness"] = aura.brightness.toInt();
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
			aura.brightness = RgbBrightness::fromInt(node["brightness"].as<int>());
		}

		if (node["config"]) {
			aura.config = node["config"].as<std::unordered_map<std::string, EffectConfig>>();
		} else if (node["effects"]) {
			aura.config = node["effects"].as<std::unordered_map<std::string, EffectConfig>>();
		}

		if (node["effect"] && !node["effect"].IsNull()) {
			aura.last_effect = node["effect"].as<std::string>();
		} else if (node["last_effect"] && !node["last_effect"].IsNull()) {
			aura.last_effect = node["last_effect"].as<std::string>();
		} else {
			aura.last_effect = std::nullopt;
		}

		return true;
	}
};
}  // namespace YAML