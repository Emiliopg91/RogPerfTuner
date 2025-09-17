#pragma once

#include <array>

#include "../base/str_enum.hpp"
// --------------------
// Meta independiente
// --------------------
struct MangoHudLevelMeta {
	enum class Enum { NO_DISPLAY, FPS_ONLY, HORIZONTAL_VIEW, EXTENDED, HIGH_DETAILED } e;
	const char* name;
	const char* val;
};

class MangoHudLevel : public StrEnum<MangoHudLevel, MangoHudLevelMeta::Enum, 5> {
  public:
	using Enum = MangoHudLevelMeta::Enum;
	using Base = StrEnum<MangoHudLevel, Enum, 5>;
	using Base::Base;

	int getPresetIndex() {
		switch (this->value) {
			case MangoHudLevelMeta::Enum::FPS_ONLY:
				return 1;
			case MangoHudLevelMeta::Enum::HORIZONTAL_VIEW:
				return 2;
			case MangoHudLevelMeta::Enum::EXTENDED:
				return 3;
			case MangoHudLevelMeta::Enum::HIGH_DETAILED:
				return 4;
			default:
				return 0;
		}
	}

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
