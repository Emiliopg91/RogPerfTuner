#pragma once

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"

enum class ComputerType : int { COMPUTER = 0, STEAM_DECK = 1, /*STEAM_MACHINE*/ };

namespace ComputerTypeNS {
constexpr auto values() {
	return EnumUtils<ComputerType>::values();
}

constexpr std::string toName(ComputerType profile) {
	return std::string(EnumUtils<ComputerType>::toString(profile));
}

constexpr std::string toString(ComputerType dev) {
	return StringUtils::toLowerCase(toName(dev));
}

inline ComputerType fromString(std::string s) {
	std::optional<ComputerType> v = EnumUtils<ComputerType>::fromString(StringUtils::replace(StringUtils::toUpperCase(s), "-", "_"));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid ComputerType " + s;
}
}  // namespace ComputerTypeNS