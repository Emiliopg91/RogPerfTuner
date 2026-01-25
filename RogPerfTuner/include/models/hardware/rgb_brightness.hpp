#pragma once

#include <algorithm>

#include "../../utils/enum_utils.hpp"

enum class RgbBrightness : int { MAX = 100, HIGH = 67, MEDIUM = 50, LOW = 33, OFF = 0 };

namespace RgbBrightnessNS {
constexpr auto values() {
	auto v = EnumUtils<RgbBrightness>::values();
	std::reverse(v.begin(), v.end());
	return v;
}

constexpr auto toInt(RgbBrightness level) {
	return EnumUtils<RgbBrightness>::toInt(level);
}

constexpr auto toName(RgbBrightness level) {
	return std::string(EnumUtils<RgbBrightness>::toString(level));
}

inline RgbBrightness fromInt(int i) {
	std::optional<RgbBrightness> v = EnumUtils<RgbBrightness>::fromInt(i);

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid RgbBrightness " + std::to_string(i);
}

inline RgbBrightness getNextBrightness(RgbBrightness value) {
	switch (value) {
		case RgbBrightness::MEDIUM:
			return RgbBrightness(RgbBrightness::HIGH);
		case RgbBrightness::LOW:
			return RgbBrightness(RgbBrightness::MEDIUM);
		case RgbBrightness::OFF:
			return RgbBrightness(RgbBrightness::LOW);
		default:
			return RgbBrightness::MAX;
	}
}

inline RgbBrightness getPreviousBrightness(RgbBrightness value) {
	switch (value) {
		case RgbBrightness::MAX:
			return RgbBrightness(RgbBrightness::HIGH);
		case RgbBrightness::HIGH:
			return RgbBrightness(RgbBrightness::MEDIUM);
		case RgbBrightness::MEDIUM:
			return RgbBrightness(RgbBrightness::LOW);
		default:
			return RgbBrightness::OFF;
	}
}
}  // namespace RgbBrightnessNS