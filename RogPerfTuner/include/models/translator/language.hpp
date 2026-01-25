#pragma once

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"
enum class Language : int { DE, EN, ES };

namespace LanguageNS {

constexpr std::string toName(Language profile) {
	return std::string(EnumUtils<Language>::toString(profile));
}

constexpr std::string toString(Language dev) {
	return StringUtils::toLowerCase(toName(dev));
}

inline Language fromString(std::string s) {
	std::optional<Language> v = EnumUtils<Language>::fromString(StringUtils::toUpperCase(s));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid MangoHudLevel " + s;
}
}  // namespace LanguageNS