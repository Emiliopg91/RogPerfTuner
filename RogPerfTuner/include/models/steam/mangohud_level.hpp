#pragma once

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"

enum class MangoHudLevel : int { NO_DISPLAY = 0, FPS_ONLY = 1, HORIZONTAL = 2, EXTENDED = 3, DETAILED = 4 };

namespace MangoHudLevelNS {
constexpr auto values() {
	return EnumUtils<MangoHudLevel>::values();
}
constexpr std::string toName(MangoHudLevel profile) {
	return std::string(EnumUtils<MangoHudLevel>::toString(profile));
}

constexpr std::string toString(MangoHudLevel dev) {
	return StringUtils::toLowerCase(toName(dev));
}

constexpr auto toInt(MangoHudLevel level) {
	return EnumUtils<MangoHudLevel>::toInt(level);
}

inline MangoHudLevel fromString(std::string s) {
	std::optional<MangoHudLevel> v = EnumUtils<MangoHudLevel>::fromString(StringUtils::toUpperCase(s));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid MangoHudLevel " + s;
}
}  // namespace MangoHudLevelNS

/*public StrEnum<MangoHudLevel, MangoHudLevelMeta::Enum, 5> {
  public:
	using Enum = MangoHudLevelMeta::Enum;
	using Base = StrEnum<MangoHudLevel, Enum, 5>;
	using Base::Base;

	int getPresetIndex();

  private:
	static constexpr std::array<MangoHudLevelMeta, 5> table{{{Enum::NO_DISPLAY, "NO_DISPLAY", "no_display"},
															 {Enum::FPS_ONLY, "FPS_ONLY", "fps_only"},
															 {Enum::HORIZONTAL_VIEW, "HORIZONTAL_VIEW", "horizontal"},
															 {Enum::EXTENDED, "EXTENDED", "extended"},
															 {Enum::HIGH_DETAILED, "HIGH_DETAILED", "detailed"}}};

	static constexpr const std::array<MangoHudLevelMeta, 5>& metaTable() {
		return table;
	}

	friend Base;
};
*/