#include "models/performance/platform_profile.hpp"

std::string formatValue(PlatformProfile profile) {
	switch (profile) {
		case PlatformProfile::LOW_POWER:
			return "Quiet";
		case PlatformProfile::BALANCED:
			return "Balanced";
		case PlatformProfile::PERFORMANCE:
			return "Performance";
	}
}