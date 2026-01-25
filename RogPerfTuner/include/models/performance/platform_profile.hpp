#pragma once
#include "../../utils/enum_utils.hpp"

enum class PlatformProfile : int { BALANCED = 0, PERFORMANCE = 1, LOW_POWER = 3 };

namespace PlatformProfileNS {
constexpr std::array<PlatformProfile, 3> values() {
	return EnumUtils<PlatformProfile>::values();
}
constexpr std::string toName(PlatformProfile profile) {
	return std::string(EnumUtils<PlatformProfile>::toString(profile));
}

constexpr auto toInt(PlatformProfile profile) {
	return EnumUtils<PlatformProfile>::toInt(profile);
}

inline PlatformProfile fromInt(int i) {
	std::optional<PlatformProfile> v = EnumUtils<PlatformProfile>::fromInt(i);

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid PlatformProfile " + std::to_string(i);
}

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
}  // namespace PlatformProfileNS