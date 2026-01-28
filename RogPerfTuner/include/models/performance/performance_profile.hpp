#pragma once

#include "models/performance/platform_profile.hpp"
#include "models/performance/power_profile.hpp"

enum class PerformanceProfile { PERFORMANCE, BALANCED, QUIET };

bool supportedOnBattery(PerformanceProfile value);

PerformanceProfile getNextPerformanceProfile(PerformanceProfile value);

PerformanceProfile getGreater(const PerformanceProfile value, const PerformanceProfile& other);

PlatformProfile getPlatformProfile(const PerformanceProfile value);

PowerProfile getPowerProfile(const PerformanceProfile value);