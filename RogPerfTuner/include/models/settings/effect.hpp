#pragma once

#include <yaml-cpp/yaml.h>

#include <string>

struct EffectConfig {
	std::string color;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<EffectConfig> {
	static Node encode(const EffectConfig& effect) {
		Node node;
		node["color"] = effect.color;
		return node;
	}

	static bool decode(const Node& node, EffectConfig& effect) {
		if (node["color"]) {
			effect.color = node["color"].as<std::string>();
		}
		return true;
	}
};
}  // namespace YAML