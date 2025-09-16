#pragma once

#include <array>

#include "../base/int_enum.hpp"
#include "../performance/platform_profile.hpp"
#include "../performance/power_profile.hpp"
#include "../performance/ssd_scheduler.hpp"

struct PerformanceProfileMeta {
	enum class Enum : int { PERFORMANCE = 2, BALANCED = 1, QUIET = 0 } e;
	const char* name;
	int val;
};

class PerformanceProfile : public IntEnum<PerformanceProfile, PerformanceProfileMeta::Enum, PerformanceProfileMeta, 3> {
  public:
	using Enum = PerformanceProfileMeta::Enum;
	using Base = IntEnum<PerformanceProfile, Enum, PerformanceProfileMeta, 3>;
	using Base::Base;

	PerformanceProfile getNextPerformanceProfile() {
		if (*this == PerformanceProfile::Enum::PERFORMANCE) {
			return PerformanceProfile::Enum::QUIET;
		}
		if (*this == PerformanceProfile::Enum::BALANCED) {
			return PerformanceProfile::Enum::PERFORMANCE;
		}
		if (*this == PerformanceProfile::Enum::QUIET) {
			return PerformanceProfile::Enum::BALANCED;
		}
		return *this;
	}

	PerformanceProfile getGreater(const PerformanceProfile& other) {
		if (*this == PerformanceProfile::Enum::PERFORMANCE || other == PerformanceProfile::Enum::PERFORMANCE) {
			return PerformanceProfile::Enum::PERFORMANCE;
		}
		if (*this == PerformanceProfile::Enum::BALANCED || other == PerformanceProfile::Enum::BALANCED) {
			return PerformanceProfile::Enum::BALANCED;
		}
		return PerformanceProfile::Enum::QUIET;
	}

	PlatformProfile getPlatformProfile() {
		if (*this == PerformanceProfile::Enum::QUIET) {
			return PlatformProfile::Enum::LOW_POWER;
		} else if (*this == PerformanceProfile::Enum::BALANCED) {
			return PlatformProfile::Enum::BALANCED;
		} else {
			return PlatformProfile::Enum::PERFORMANCE;
		}
	}

	PowerProfile getPowerProfile() {
		if (*this == PerformanceProfile::Enum::QUIET) {
			return PowerProfile::Enum::POWERSAVER;
		} else if (*this == PerformanceProfile::Enum::BALANCED) {
			return PowerProfile::Enum::BALANCED;
		} else {
			return PowerProfile::Enum::PERFORMANCE;
		}
	}

	SsdScheduler getSsdQueueScheduler() {
		if (*this == PerformanceProfile::Enum::QUIET) {
			return SsdScheduler::Enum::NOOP;
		} else {
			return SsdScheduler::Enum::MQ_DEADLINE;
		}
	}

  private:
	static constexpr std::array<PerformanceProfileMeta, 3> table{
		{{Enum::QUIET, "QUIET", 0}, {Enum::BALANCED, "BALANCED", 1}, {Enum::PERFORMANCE, "PERFORMANCE", 2}}};

	static constexpr const std::array<PerformanceProfileMeta, 3>& metaTable() {
		return table;
	}

	friend Base;
};
