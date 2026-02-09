#pragma once

enum class RgbBrightness { MAX = 100, HIGH = 67, MEDIUM = 50, LOW = 33, OFF = 0 };

inline RgbBrightness getNextBrightness(RgbBrightness value) {
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

inline RgbBrightness getPreviousBrightness(RgbBrightness value) {
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