#pragma once

#include <array>

#include "../base/int_enum.hpp"

struct PlatformProfileMeta {
	enum class Enum : int { BALANCED = 0, PERFORMANCE = 1, LOW_POWER = 3 } e;
	const char* name;
	int val;
};

class PlatformProfile : public IntEnum<PlatformProfile, PlatformProfileMeta::Enum, PlatformProfileMeta, 3> {
  public:
	using Enum = PlatformProfileMeta::Enum;
	using Base = IntEnum<PlatformProfile, Enum, PlatformProfileMeta, 3>;
	using Base::Base;

	std::string formatValue() {
		switch (this->value) {
			case PlatformProfileMeta::Enum::LOW_POWER:
				return "Quiet";
			case PlatformProfileMeta::Enum::BALANCED:
				return "Balanced";
			case PlatformProfileMeta::Enum::PERFORMANCE:
				return "Performance";
		}
	}

  private:
	static constexpr std::array<PlatformProfileMeta, 3> table{
		{{Enum::BALANCED, "BALANCED", 0}, {Enum::PERFORMANCE, "PERFORMANCE", 1}, {Enum::LOW_POWER, "LOW_POWER", 3}}};

	static constexpr const std::array<PlatformProfileMeta, 3>& metaTable() {
		return table;
	}

	friend Base;
};
