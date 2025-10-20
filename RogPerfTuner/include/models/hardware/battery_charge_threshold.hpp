#pragma once

#include <array>

#include "../base/int_enum.hpp"
// --------------------
// Meta independiente
// --------------------
struct BatteryThresholdMeta {
	enum class Enum : int { CT_100 = 100, CT_75 = 75, CT_50 = 50 } e;
	const char* name;
	int val;
};

class BatteryThreshold : public IntEnum<BatteryThreshold, BatteryThresholdMeta::Enum, BatteryThresholdMeta, 3> {
  public:
	using Enum = BatteryThresholdMeta::Enum;
	using Base = IntEnum<BatteryThreshold, Enum, BatteryThresholdMeta, 3>;
	using Base::Base;

  private:
	static constexpr std::array<BatteryThresholdMeta, 3> table{
		{{Enum::CT_100, "CT_100", 100}, {Enum::CT_75, "CT_75", 75}, {Enum::CT_50, "CT_50", 50}}};

	static constexpr const std::array<BatteryThresholdMeta, 3>& metaTable() {
		return table;
	}

	friend Base;
};
