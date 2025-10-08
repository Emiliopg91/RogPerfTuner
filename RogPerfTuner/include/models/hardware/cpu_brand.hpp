#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct CpuBrandMeta {
	enum class Enum { INTEL } e;
	const char* name;
	const char* val;
};

class CpuBrand : public StrEnum<CpuBrand, CpuBrandMeta::Enum, 3> {
  public:
	using Enum = CpuBrandMeta::Enum;
	using Base = StrEnum<CpuBrand, Enum, 3>;
	using Base::Base;

  private:
	static constexpr std::array<CpuBrandMeta, 3> table{{{Enum::INTEL, "INTEL", "intel"}}};

	friend Base;
	static constexpr const std::array<CpuBrandMeta, 3>& metaTable() {
		return table;
	}
};
