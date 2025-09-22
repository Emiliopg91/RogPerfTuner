#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../hardware/battery_charge_threshold.hpp"
#include "performance.hpp"

struct Platform {
	Performance performance																	= Performance{};
	BatteryThreshold chargeLimit															= BatteryThreshold::Enum::CT_100;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> fanCurves = {};
};

inline void to_json(nlohmann::json& j, const Platform& o) {
	j				 = nlohmann::json{};
	j["performance"] = o.performance;
	j["chargeLimit"] = o.chargeLimit.toInt();
	j["fanCurves"]	 = o.fanCurves;
}
inline void from_json(const nlohmann::json& j, Platform& o) {
	o.performance = j.at("performance").get<Performance>();
	if (j.contains("chargeLimit")) {
		o.chargeLimit = BatteryThreshold::fromInt(j.at("chargeLimit").get<int>());
	}
	if (j.contains("fanCurves")) {
		o.fanCurves = j.at("fanCurves").get<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>();
	}
}