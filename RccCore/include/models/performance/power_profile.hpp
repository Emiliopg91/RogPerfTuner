#pragma once

#include "../base/str_enum.hpp"

struct PowerProfileMeta {
	enum class Enum { BALANCED, PERFORMANCE, POWERSAVER } e;
	const char* name;
	const char* val;
};

class PowerProfile : public StrEnum<PowerProfile, PowerProfileMeta::Enum, 3> {
  public:
	using Enum = PowerProfileMeta::Enum;
	using Base = StrEnum<PowerProfile, Enum, 3>;
	using Base::Base;

  private:
	static constexpr std::array<PowerProfileMeta, 3> table{{{Enum::BALANCED, "BALANCED", "balanced"},
															{Enum::PERFORMANCE, "PERFORMANCE", "performance"},
															{Enum::POWERSAVER, "POWERSAVER", "power-saver"}}};

	friend Base;
	static constexpr const std::array<PowerProfileMeta, 3>& metaTable() {
		return table;
	}
};
