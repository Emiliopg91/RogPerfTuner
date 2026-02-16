#pragma once

#include <string>

enum class PlatformProfile { BALANCED = 0, PERFORMANCE = 1, QUIET = 2 };

inline std::string formatValue(PlatformProfile profile) {
	switch (profile) {
		case PlatformProfile::QUIET:
			return "Quiet";
		case PlatformProfile::BALANCED:
			return "Balanced";
		case PlatformProfile::PERFORMANCE:
			return "Performance";
	}
}