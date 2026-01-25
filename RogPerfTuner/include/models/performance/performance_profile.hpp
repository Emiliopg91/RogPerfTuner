#pragma once
#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"
#include "../performance/platform_profile.hpp"
#include "../performance/power_profile.hpp"

enum class PerformanceProfile : int { PERFORMANCE = 0, BALANCED = 1, QUIET = 2 };

namespace PerformanceProfileNS {
constexpr std::array<PerformanceProfile, 3> values() {
	return EnumUtils<PerformanceProfile>::values();
}

constexpr std::string toName(PerformanceProfile profile) {
	return std::string(EnumUtils<PerformanceProfile>::toString(profile));
}

constexpr std::string toString(PerformanceProfile profile) {
	return StringUtils::toLowerCase(toName(profile));
}

inline PerformanceProfile fromString(std::string s) {
	std::optional<PerformanceProfile> v = EnumUtils<PerformanceProfile>::fromString(StringUtils::toUpperCase(s));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid PerformanceProfile " + s;
}

inline bool supportedOnBattery(PerformanceProfile value) {
	return value == PerformanceProfile::QUIET;
}

inline PerformanceProfile getNextPerformanceProfile(PerformanceProfile value) {
	if (value == PerformanceProfile::PERFORMANCE) {
		return PerformanceProfile::QUIET;
	}
	if (value == PerformanceProfile::BALANCED) {
		return PerformanceProfile::PERFORMANCE;
	}
	if (value == PerformanceProfile::QUIET) {
		return PerformanceProfile::BALANCED;
	}
	return value;
}

inline PerformanceProfile getGreater(const PerformanceProfile value, const PerformanceProfile& other) {
	if (value == PerformanceProfile::PERFORMANCE || other == PerformanceProfile::PERFORMANCE) {
		return PerformanceProfile::PERFORMANCE;
	}
	if (value == PerformanceProfile::BALANCED || other == PerformanceProfile::BALANCED) {
		return PerformanceProfile::BALANCED;
	}
	return PerformanceProfile::QUIET;
}

inline PlatformProfile getPlatformProfile(const PerformanceProfile value) {
	if (value == PerformanceProfile::QUIET) {
		return PlatformProfile::LOW_POWER;
	} else if (value == PerformanceProfile::BALANCED) {
		return PlatformProfile::BALANCED;
	} else {
		return PlatformProfile::PERFORMANCE;
	}
}

inline PowerProfile getPowerProfile(const PerformanceProfile value) {
	if (value == PerformanceProfile::QUIET) {
		return PowerProfile::POWER_SAVER;
	} else if (value == PerformanceProfile::BALANCED) {
		return PowerProfile::BALANCED;
	} else {
		return PowerProfile::PERFORMANCE;
	}
}

}  // namespace PerformanceProfileNS