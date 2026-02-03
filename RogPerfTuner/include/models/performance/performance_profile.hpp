#pragma once

#include "models/performance/platform_profile.hpp"
#include "models/performance/power_profile.hpp"

enum class PerformanceProfile { SMART, PERFORMANCE, BALANCED, QUIET };

PerformanceProfile getNextPerformanceProfile(PerformanceProfile value, bool circular = true);

PerformanceProfile getPreviousPerformanceProfile(PerformanceProfile value, bool circular = true);

PerformanceProfile getGreater(const PerformanceProfile value, const PerformanceProfile& other);

PlatformProfile getPlatformProfile(const PerformanceProfile value);

PowerProfile getPowerProfile(const PerformanceProfile value);