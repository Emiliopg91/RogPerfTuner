#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../hardware/battery_charge_threshold.hpp"
#include "performance.hpp"

struct Platform {
	Performance performance		 = Performance();
	BatteryThreshold chargeLimit = BatteryThreshold::Enum::CT_100;
};

inline void to_json(nlohmann::json& j, const Platform& o) {
	j				 = nlohmann::json{};
	j["performance"] = o.performance;
	j["chargeLimit"] = o.chargeLimit.toInt();
}
inline void from_json(const nlohmann::json& j, Platform& o) {
	o.performance = j.contains("performance") ? j.at("performance").get<Performance>() : Performance{};
	o.chargeLimit = j.contains("chargeLimit") ? BatteryThreshold::fromInt(j.at("chargeLimit").get<int>()) : BatteryThreshold::Enum::CT_100;
}