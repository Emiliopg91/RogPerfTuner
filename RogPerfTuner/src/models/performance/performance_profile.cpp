#include "../../../include/models/performance/performance_profile.hpp"

bool supportedOnBattery(PerformanceProfile value) {
	return value == PerformanceProfile::QUIET;
}

PerformanceProfile getNextPerformanceProfile(PerformanceProfile value) {
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

PerformanceProfile getGreater(const PerformanceProfile value, const PerformanceProfile& other) {
	if (value == PerformanceProfile::PERFORMANCE || other == PerformanceProfile::PERFORMANCE) {
		return PerformanceProfile::PERFORMANCE;
	}
	if (value == PerformanceProfile::BALANCED || other == PerformanceProfile::BALANCED) {
		return PerformanceProfile::BALANCED;
	}
	return PerformanceProfile::QUIET;
}

PlatformProfile getPlatformProfile(const PerformanceProfile value) {
	if (value == PerformanceProfile::QUIET) {
		return PlatformProfile::LOW_POWER;
	} else if (value == PerformanceProfile::BALANCED) {
		return PlatformProfile::BALANCED;
	} else {
		return PlatformProfile::PERFORMANCE;
	}
}

PowerProfile getPowerProfile(const PerformanceProfile value) {
	if (value == PerformanceProfile::QUIET) {
		return PowerProfile::POWER_SAVER;
	} else if (value == PerformanceProfile::BALANCED) {
		return PowerProfile::BALANCED;
	} else {
		return PowerProfile::PERFORMANCE;
	}
}