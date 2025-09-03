#pragma once

#include "../base/str_enum.hpp"

struct CpuGovernorMeta {
	enum class Enum { POWERSAVE, PERFORMANCE } e;
	const char* name;
	const char* val;
};

class CpuGovernor : public StrEnum<CpuGovernor, CpuGovernorMeta::Enum, 3> {
   public:
	using Enum = CpuGovernorMeta::Enum;
	using Base = StrEnum<CpuGovernor, Enum, 3>;
	using Base::Base;

   private:
	static constexpr std::array<CpuGovernorMeta, 3> table{
		{{Enum::POWERSAVE, "POWERSAVE", "powersave"}, {Enum::PERFORMANCE, "PERFORMANCE", "performance"}}};

	friend Base;
	static constexpr const std::array<CpuGovernorMeta, 3>& metaTable() {
		return table;
	}
};