#pragma once

#include <array>

#include "../base/int_enum.hpp"

struct RgbBrightnessMeta {
	enum class Enum : int { MAX = 100, HIGH = 67, MEDIUM = 50, LOW = 33, OFF = 0 } e;
	const char* name;
	int val;
};

class RgbBrightness : public IntEnum<RgbBrightness, RgbBrightnessMeta::Enum, RgbBrightnessMeta, 5> {
  public:
	using Enum = RgbBrightnessMeta::Enum;
	using Base = IntEnum<RgbBrightness, Enum, RgbBrightnessMeta, 5>;
	using Base::Base;

	RgbBrightness getNextBrightness() const;

	RgbBrightness getPreviousBrightness() const;

  private:
	static constexpr std::array<RgbBrightnessMeta, 5> table{
		{{Enum::MAX, "MAX", 100}, {Enum::HIGH, "HIGH", 67}, {Enum::MEDIUM, "MEDIUM", 50}, {Enum::LOW, "LOW", 33}, {Enum::OFF, "OFF", 0}}};

	static constexpr const std::array<RgbBrightnessMeta, 5>& metaTable() {
		return table;
	}

	friend Base;
};
