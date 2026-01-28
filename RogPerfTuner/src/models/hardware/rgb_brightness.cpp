#include "models/hardware/rgb_brightness.hpp"

RgbBrightness getNextBrightness(RgbBrightness value) {
	switch (value) {
		case RgbBrightness::MEDIUM:
			return RgbBrightness(RgbBrightness::HIGH);
		case RgbBrightness::LOW:
			return RgbBrightness(RgbBrightness::MEDIUM);
		case RgbBrightness::OFF:
			return RgbBrightness(RgbBrightness::LOW);
		default:
			return RgbBrightness::MAX;
	}
}

RgbBrightness getPreviousBrightness(RgbBrightness value) {
	switch (value) {
		case RgbBrightness::MAX:
			return RgbBrightness(RgbBrightness::HIGH);
		case RgbBrightness::HIGH:
			return RgbBrightness(RgbBrightness::MEDIUM);
		case RgbBrightness::MEDIUM:
			return RgbBrightness(RgbBrightness::LOW);
		default:
			return RgbBrightness::OFF;
	}
}