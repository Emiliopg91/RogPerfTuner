#pragma once

#include <array>

#include "../base/str_enum.hpp"
#include "../performance/platform_profile.hpp"
#include "../performance/power_profile.hpp"
#include "../performance/ssd_scheduler.hpp"

struct PerformanceProfileMeta {
	enum class Enum { PERFORMANCE, BALANCED, LOWPOWER } e;
	const char* name;
	const char* val;
};

class PerformanceProfile : public StrEnum<PerformanceProfile, PerformanceProfileMeta::Enum, 3> {
  public:
	using Enum = PerformanceProfileMeta::Enum;
	using Base = StrEnum<PerformanceProfile, Enum, 3>;
	using Base::Base;

	PerformanceProfile getNextPerformanceProfile();

	PerformanceProfile getGreater(const PerformanceProfile& other);

	PlatformProfile getPlatformProfile();

	PowerProfile getPowerProfile();

	SsdScheduler getSsdQueueScheduler();

  private:
	static constexpr std::array<PerformanceProfileMeta, 3> table{
		{{Enum::LOWPOWER, "LOWPOWER", "LowPower"}, {Enum::BALANCED, "BALANCED", "Balanced"}, {Enum::PERFORMANCE, "PERFORMANCE", "Performance"}}};

	static constexpr const std::array<PerformanceProfileMeta, 3>& metaTable() {
		return table;
	}

	friend Base;
};
