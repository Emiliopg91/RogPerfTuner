#pragma once

#include "models/performance/platform_profile.hpp"
#include "models/performance/power_profile.hpp"

enum class PerformanceProfile { SMART, PERFORMANCE, BALANCED, QUIET };

inline PerformanceProfile getNextPerformanceProfile(const PerformanceProfile& value, bool circular = true, bool includeSmart = true) {
	switch (value) {
		case PerformanceProfile::SMART:
			return circular ? PerformanceProfile::QUIET : PerformanceProfile::SMART;
		case PerformanceProfile::PERFORMANCE:
			return includeSmart ? PerformanceProfile::SMART : (circular ? PerformanceProfile::QUIET : PerformanceProfile::PERFORMANCE);
		case PerformanceProfile::BALANCED:
			return PerformanceProfile::PERFORMANCE;
		case PerformanceProfile::QUIET:
			return PerformanceProfile::BALANCED;
		default:
			return value;
	}
}

inline PerformanceProfile getPreviousPerformanceProfile(const PerformanceProfile& value, bool circular) {
	switch (value) {
		case PerformanceProfile::PERFORMANCE:
			return PerformanceProfile::BALANCED;
		case PerformanceProfile::BALANCED:
			return PerformanceProfile::QUIET;
		case PerformanceProfile::QUIET:
			return circular ? PerformanceProfile::PERFORMANCE : PerformanceProfile::QUIET;
		default:
			return value;
	}
}

inline PlatformProfile getPlatformProfile(const PerformanceProfile& value) {
	switch (value) {
		case PerformanceProfile::QUIET:
			return PlatformProfile::LOW_POWER;
		case PerformanceProfile::BALANCED:
			return PlatformProfile::BALANCED;
		default:
			return PlatformProfile::PERFORMANCE;
	}
}

inline PowerProfile getPowerProfile(const PerformanceProfile& value) {
	switch (value) {
		case PerformanceProfile::QUIET:
			return PowerProfile::POWER_SAVER;
		case PerformanceProfile::BALANCED:
			return PowerProfile::BALANCED;
		default:
			return PowerProfile::PERFORMANCE;
	}
}

inline PerformanceProfile getGreater(const PerformanceProfile& value, const PerformanceProfile& other) {
	if (value == PerformanceProfile::PERFORMANCE || other == PerformanceProfile::PERFORMANCE) {
		return PerformanceProfile::PERFORMANCE;
	}
	if (value == PerformanceProfile::BALANCED || other == PerformanceProfile::BALANCED) {
		return PerformanceProfile::BALANCED;
	}
	return PerformanceProfile::QUIET;
}