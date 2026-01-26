#pragma once

enum class RgbBrightness { MAX = 100, HIGH = 67, MEDIUM = 50, LOW = 33, OFF = 0 };

RgbBrightness getNextBrightness(RgbBrightness value);

RgbBrightness getPreviousBrightness(RgbBrightness value);