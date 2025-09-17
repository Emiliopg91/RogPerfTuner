#pragma once

#include <array>

#include "../base/str_enum.hpp"
#include "../performance/platform_profile.hpp"
#include "../performance/power_profile.hpp"
#include "../performance/ssd_scheduler.hpp"

struct PerformanceProfileMeta {
	enum class Enum { PERFORMANCE, BALANCED, QUIET } e;
	const char* name;
	const char* val;
};

class PerformanceProfile : public StrEnum<PerformanceProfile, PerformanceProfileMeta::Enum, 3> {
  public:
	using Enum = PerformanceProfileMeta::Enum;
	using Base = StrEnum<PerformanceProfile, Enum, 3>;
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
		{{Enum::QUIET, "QUIET", "quiet"}, {Enum::BALANCED, "BALANCED", "balanced"}, {Enum::PERFORMANCE, "PERFORMANCE", "performance"}}};

	static constexpr const std::array<PerformanceProfileMeta, 3>& metaTable() {
		return table;
	}

	friend Base;
};
