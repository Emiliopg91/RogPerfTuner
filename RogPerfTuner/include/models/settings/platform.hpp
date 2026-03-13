#pragma once

#include <yaml-cpp/yaml.h>

#include "models/hardware/battery_charge_threshold.hpp"
#include "models/settings/fan_curve.hpp"
#include "models/settings/performance.hpp"

struct Platform {
	Performance performance = Performance{};
#ifdef BAT_LIMIT
	BatteryThreshold chargeLimit = BatteryThreshold::CT_100;
#endif
#ifdef FAN_CONTROL
	std::map<std::string, std::map<std::string, FanCurve>> curves = {};
#endif
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Platform> {
	static Node encode(const Platform& platform) {
		Node node;
#ifdef BAT_LIMIT
		node["chargeLimit"] = toInt(platform.chargeLimit);
#endif
#ifdef FAN_CONTROL
		if (!platform.curves.empty()) {
			node["curves"] = platform.curves;
		}
#endif
		node["performance"] = platform.performance;
		return node;
	}

	static bool decode(const Node& node, Platform& platform) {
		if (node["performance"]) {
			platform.performance = node["performance"].as<Performance>();
		}
#ifdef BAT_LIMIT
		if (node["chargeLimit"]) {
			platform.chargeLimit = fromInt<BatteryThreshold>(node["chargeLimit"].as<int>());
		}
#endif
#ifdef FAN_CONTROL
		if (node["curves"]) {
			platform.curves = node["curves"].as<std::map<std::string, std::map<std::string, FanCurve>>>();
		}
#endif
		return true;
	}
};
}  // namespace YAML