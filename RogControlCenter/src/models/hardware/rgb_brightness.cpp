#include "../../../include/models/hardware/rgb_brightness.hpp"

RgbBrightness RgbBrightness::getNextBrightness() const {
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

RgbBrightness RgbBrightness::getPreviousBrightness() const {
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