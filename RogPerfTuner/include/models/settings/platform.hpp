#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "../hardware/battery_charge_threshold.hpp"
#include "fan_curve.hpp"
#include "performance.hpp"

struct Platform {
	Performance performance															  = Performance{};
	BatteryThreshold chargeLimit													  = BatteryThreshold::Enum::CT_100;
	std::unordered_map<std::string, std::unordered_map<std::string, FanCurve>> curves = {};
};

inline void to_json(nlohmann::json& j, const Platform& o) {
	j				 = nlohmann::json{};
	j["performance"] = o.performance;
	j["chargeLimit"] = o.chargeLimit.toInt();
	if (!o.curves.empty()) {
		j["curves"] = o.curves;
	}
}
inline void from_json(const nlohmann::json& j, Platform& o) {
	o.performance = j.at("performance").get<Performance>();
	if (j.contains("chargeLimit")) {
		o.chargeLimit = BatteryThreshold::fromInt(j.at("chargeLimit").get<int>());
	}
	if (j.contains("curves")) {
		o.curves = j.at("curves").get<std::unordered_map<std::string, std::unordered_map<std::string, FanCurve>>>();
	}
}