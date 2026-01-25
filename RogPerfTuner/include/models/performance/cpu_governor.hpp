#pragma once
#include "../../utils/enum_utils.hpp"
#include "../../utils/string_utils.hpp"

enum class CpuGovernor : int { POWERSAVE, PERFORMANCE };

namespace CpuGovernorNS {
constexpr std::string toName(CpuGovernor governor) {
	return std::string(EnumUtils<CpuGovernor>::toString(governor));
}

constexpr std::string toString(CpuGovernor governor) {
	return StringUtils::toLowerCase(toName(governor));
}
}  // namespace CpuGovernorNS