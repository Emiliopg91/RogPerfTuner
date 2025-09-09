#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct GpuBrandMeta {
	enum class Enum { INTEL, NVIDIA } e;
	const char* name;
	const char* val;
};

class GpuBrand : public StrEnum<GpuBrand, GpuBrandMeta::Enum, 3> {
  public:
	using Enum = GpuBrandMeta::Enum;
	using Base = StrEnum<GpuBrand, Enum, 3>;
	using Base::Base;

  private:
	static constexpr std::array<GpuBrandMeta, 3> table{{{Enum::INTEL, "INTEL", "intel"}, {Enum::NVIDIA, "NVIDIA", "nvidia"}}};

	friend Base;
	static constexpr const std::array<GpuBrandMeta, 3>& metaTable() {
		return table;
	}
};
