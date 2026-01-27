#pragma once

#include <string>

#include "../../utils/enum_utils.hpp"

enum class GpuBrand { INTEL, NVIDIA, AMD };

inline std::string getIcdName(GpuBrand brand) {
	switch (brand) {
		case GpuBrand::AMD:
			return "radeon";
		default:
			return toString(brand);
	}
}

inline std::string getOclName(GpuBrand brand) {
	switch (brand) {
		case GpuBrand::AMD:
			return "amdocl64";
		default:
			return toString(brand);
	}
}