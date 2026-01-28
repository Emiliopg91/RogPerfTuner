#pragma once

#include <yaml-cpp/yaml.h>

#include "models/hardware/battery_charge_threshold.hpp"
#include "models/settings/fan_curve.hpp"
#include "models/settings/performance.hpp"

struct Platform {
	Performance performance										  = Performance{};
	BatteryThreshold chargeLimit								  = BatteryThreshold::CT_100;
	std::map<std::string, std::map<std::string, FanCurve>> curves = {};
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Platform> {
	static Node encode(const Platform& platform) {
		Node node;
		node["chargeLimit"] = toInt(platform.chargeLimit);
		if (!platform.curves.empty()) {
			node["curves"] = platform.curves;
		}
		node["performance"] = platform.performance;
		return node;
	}

	static bool decode(const Node& node, Platform& platform) {
		if (node["performance"]) {
			platform.performance = node["performance"].as<Performance>();
		}
		if (node["chargeLimit"]) {
			platform.chargeLimit = fromInt<BatteryThreshold>(node["chargeLimit"].as<int>());
		}
		if (node["curves"]) {
			platform.curves = node["curves"].as<std::map<std::string, std::map<std::string, FanCurve>>>();
		}
		return true;
	}
};
}  // namespace YAML