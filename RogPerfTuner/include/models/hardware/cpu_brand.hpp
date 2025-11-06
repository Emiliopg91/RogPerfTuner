#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct CpuBrandMeta {
	enum class Enum { INTEL, AMD } e;
	const char* name;
	const char* val;
};

class CpuBrand : public StrEnum<CpuBrand, CpuBrandMeta::Enum, 2> {
  public:
	using Enum = CpuBrandMeta::Enum;
	using Base = StrEnum<CpuBrand, Enum, 2>;
	using Base::Base;

  private:
	static constexpr std::array<CpuBrandMeta, 2> table{{{Enum::INTEL, "INTEL", "intel"}, {Enum::AMD, "AMD", "amd"}}};

	friend Base;
	static constexpr const std::array<CpuBrandMeta, 2>& metaTable() {
		return table;
	}
};
