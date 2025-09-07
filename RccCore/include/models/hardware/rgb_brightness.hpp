#pragma once

#include "../base/int_enum.hpp"

struct RgbBrightnessMeta {
	enum class Enum : int { MAX = 4, HIGH = 3, MEDIUM = 2, LOW = 1, OFF = 0 } e;
	const char* name;
	int val;
};

class RgbBrightness : public IntEnum<RgbBrightness, RgbBrightnessMeta::Enum, RgbBrightnessMeta, 5> {
  public:
	using Enum = RgbBrightnessMeta::Enum;
	using Base = IntEnum<RgbBrightness, Enum, RgbBrightnessMeta, 5>;
	using Base::Base;

	RgbBrightness getNextBrightness() const {
		switch (this->value) {
			case Enum::HIGH:
				return RgbBrightness(Enum::MAX);
			case Enum::MEDIUM:
				return RgbBrightness(Enum::HIGH);
			case Enum::LOW:
				return RgbBrightness(Enum::MEDIUM);
			case Enum::OFF:
				return RgbBrightness(Enum::LOW);
			default:
				return *this;
		}
	}

	RgbBrightness getPreviousBrightness() const {
		switch (this->value) {
			case Enum::MAX:
				return RgbBrightness(Enum::HIGH);
			case Enum::HIGH:
				return RgbBrightness(Enum::MEDIUM);
			case Enum::MEDIUM:
				return RgbBrightness(Enum::LOW);
			case Enum::LOW:
				return RgbBrightness(Enum::OFF);
			default:
				return *this;
		}
	}

  private:
	static constexpr std::array<RgbBrightnessMeta, 5> table{
		{{Enum::MAX, "MAX", 4}, {Enum::HIGH, "HIGH", 3}, {Enum::MEDIUM, "MEDIUM", 2}, {Enum::LOW, "LOW", 1}, {Enum::OFF, "OFF", 0}}};

	static constexpr const std::array<RgbBrightnessMeta, 5>& metaTable() {
		return table;
	}

	friend Base;
};
