#include "../../../include/models/performance/performance_profile.hpp"

PerformanceProfile PerformanceProfile::getNextPerformanceProfile() {
	if (*this == PerformanceProfile::Enum::PERFORMANCE) {
		return PerformanceProfile::Enum::LOWPOWER;
	}
	if (*this == PerformanceProfile::Enum::BALANCED) {
		return PerformanceProfile::Enum::PERFORMANCE;
	}
	if (*this == PerformanceProfile::Enum::LOWPOWER) {
		return PerformanceProfile::Enum::BALANCED;
	}
	return *this;
}

PerformanceProfile PerformanceProfile::getGreater(const PerformanceProfile& other) {
	if (*this == PerformanceProfile::Enum::PERFORMANCE || other == PerformanceProfile::Enum::PERFORMANCE) {
		return PerformanceProfile::Enum::PERFORMANCE;
	}
	if (*this == PerformanceProfile::Enum::BALANCED || other == PerformanceProfile::Enum::BALANCED) {
		return PerformanceProfile::Enum::BALANCED;
	}
	return PerformanceProfile::Enum::LOWPOWER;
}

PlatformProfile PerformanceProfile::getPlatformProfile() {
	if (*this == PerformanceProfile::Enum::LOWPOWER) {
		return PlatformProfile::Enum::LOW_POWER;
	} else if (*this == PerformanceProfile::Enum::BALANCED) {
		return PlatformProfile::Enum::BALANCED;
	} else {
		return PlatformProfile::Enum::PERFORMANCE;
	}
}

PowerProfile PerformanceProfile::getPowerProfile() {
	if (*this == PerformanceProfile::Enum::LOWPOWER) {
		return PowerProfile::Enum::POWERSAVER;
	} else if (*this == PerformanceProfile::Enum::BALANCED) {
		return PowerProfile::Enum::BALANCED;
	} else {
		return PowerProfile::Enum::PERFORMANCE;
	}
}

SsdScheduler PerformanceProfile::getSsdQueueScheduler() {
	if (*this == PerformanceProfile::Enum::LOWPOWER) {
		return SsdScheduler::Enum::NOOP;
	} else {
		return SsdScheduler::Enum::MQ_DEADLINE;
	}
}