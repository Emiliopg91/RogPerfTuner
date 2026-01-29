#include "models/performance/performance_profile.hpp"

#include "models/performance/platform_profile.hpp"

PerformanceProfile getNextPerformanceProfile(PerformanceProfile value) {
	switch (value) {
		case PerformanceProfile::PERFORMANCE:
			return PerformanceProfile::QUIET;
		case PerformanceProfile::BALANCED:
			return PerformanceProfile::QUIET;
		case PerformanceProfile::QUIET:
			return PerformanceProfile::BALANCED;
		default:
			return value;
	}
}

PlatformProfile getPlatformProfile(const PerformanceProfile value) {
	switch (value) {
		case PerformanceProfile::QUIET:
			return PlatformProfile::LOW_POWER;
		case PerformanceProfile::BALANCED:
			return PlatformProfile::BALANCED;
		default:
			return PlatformProfile::PERFORMANCE;
	}
}

PowerProfile getPowerProfile(const PerformanceProfile value) {
	switch (value) {
		case PerformanceProfile::QUIET:
			return PowerProfile::POWER_SAVER;
		case PerformanceProfile::BALANCED:
			return PowerProfile::BALANCED;
		default:
			return PowerProfile::PERFORMANCE;
	}
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