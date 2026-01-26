#pragma once

#include <string>

enum class PlatformProfile : int { BALANCED = 0, PERFORMANCE = 1, LOW_POWER = 3 };

std::string formatValue(PlatformProfile profile);