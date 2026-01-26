#pragma once

#include "../performance/platform_profile.hpp"
#include "../performance/power_profile.hpp"

enum class PerformanceProfile : int { PERFORMANCE = 0, BALANCED = 1, QUIET = 2 };

bool supportedOnBattery(PerformanceProfile value);

PerformanceProfile getNextPerformanceProfile(PerformanceProfile value);

PerformanceProfile getGreater(const PerformanceProfile value, const PerformanceProfile& other);

PlatformProfile getPlatformProfile(const PerformanceProfile value);

PowerProfile getPowerProfile(const PerformanceProfile value);