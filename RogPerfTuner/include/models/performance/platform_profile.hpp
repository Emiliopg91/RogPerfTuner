#pragma once

#include <string>

enum class PlatformProfile { BALANCED = 0, PERFORMANCE = 1, LOW_POWER = 3 };

inline std::string formatValue(PlatformProfile profile) {
	switch (profile) {
		case PlatformProfile::LOW_POWER:
			return "Quiet";
		case PlatformProfile::BALANCED:
			return "Balanced";
		case PlatformProfile::PERFORMANCE:
			return "Performance";
	}
}