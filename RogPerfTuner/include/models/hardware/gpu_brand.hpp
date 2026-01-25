#pragma once
#include <string>

#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"

enum class GpuBrand : int { INTEL = 0, NVIDIA = 1, AMD = 2 };

namespace GpuBrandNS {
constexpr std::string toString(GpuBrand brand) {
	return StringUtils::toLowerCase(std::string(EnumUtils<GpuBrand>::toString(brand)));
}

inline GpuBrand fromString(std::string s) {
	std::optional<GpuBrand> v = EnumUtils<GpuBrand>::fromString(StringUtils::toUpperCase(s));

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid GpuBrand " + s;
}
}  // namespace GpuBrandNS