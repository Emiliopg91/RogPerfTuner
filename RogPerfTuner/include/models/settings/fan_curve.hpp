#pragma once

#include <yaml-cpp/yaml.h>

struct FanCurve {
	std::string current = {};
	std::string factory = {};
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<FanCurve> {
	static Node encode(const FanCurve& fanCurve) {
		Node node;
		node["current"] = fanCurve.current;
		node["factory"] = fanCurve.factory;
		return node;
	}

	static bool decode(const Node& node, FanCurve& fanCurve) {
		if (node["current"]) {
			fanCurve.current = node["current"].as<std::string>();
		}
		if (node["factory"]) {
			fanCurve.factory = node["factory"].as<std::string>();
		}
		return true;
	}
};
}  // namespace YAML