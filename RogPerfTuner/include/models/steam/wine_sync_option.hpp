#pragma once

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"

enum class WineSyncOption : int { AUTO, NTSYNC, FSYNC, ESYNC, NONE };

namespace WineSyncOptionNS {
constexpr auto values() {
	return EnumUtils<WineSyncOption>::values();
}
constexpr std::string toName(WineSyncOption profile) {
	return std::string(EnumUtils<WineSyncOption>::toString(profile));
}

constexpr std::string toString(WineSyncOption dev) {
	return StringUtils::toLowerCase(toName(dev));
}

inline WineSyncOption fromString(std::string s) {
	std::optional<WineSyncOption> v = EnumUtils<WineSyncOption>::fromString(StringUtils::toUpperCase(s));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid WineSyncOption " + s;
}
}  // namespace WineSyncOptionNS