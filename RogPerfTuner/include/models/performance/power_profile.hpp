#pragma once

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"
enum class PowerProfile : int { BALANCED, PERFORMANCE, POWER_SAVER };

namespace PowerProfileNS {

constexpr std::string toName(PowerProfile profile) {
	return std::string(EnumUtils<PowerProfile>::toString(profile));
}

constexpr std::string toString(PowerProfile profile) {
	return StringUtils::replace(StringUtils::toLowerCase(toName(profile)), "_", "-");
}

inline PowerProfile fromString(std::string s) {
	std::optional<PowerProfile> v = EnumUtils<PowerProfile>::fromString(StringUtils::replace(StringUtils::toUpperCase(s), "-", "_"));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid GpuBrand " + s;
}
}  // namespace PowerProfileNS