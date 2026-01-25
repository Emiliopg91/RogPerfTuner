#pragma once
#include <algorithm>

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"
enum class BatteryThreshold : int { CT_100 = 100, CT_75 = 75, CT_50 = 50 };

namespace BatteryThresholdNS {
constexpr auto values() {
	auto v = EnumUtils<BatteryThreshold>::values();
	std::reverse(v.begin(), v.end());
	return v;
}

constexpr auto toInt(BatteryThreshold level) {
	return EnumUtils<BatteryThreshold>::toInt(level);
}

inline BatteryThreshold fromInt(int i) {
	std::optional<BatteryThreshold> v = EnumUtils<BatteryThreshold>::fromInt(i);

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid BatteryThreshold " + std::to_string(i);
}
constexpr std::string toName(BatteryThreshold profile) {
	return std::string(EnumUtils<BatteryThreshold>::toString(profile));
}

constexpr std::string toString(BatteryThreshold dev) {
	return StringUtils::toLowerCase(toName(dev));
}
}  // namespace BatteryThresholdNS