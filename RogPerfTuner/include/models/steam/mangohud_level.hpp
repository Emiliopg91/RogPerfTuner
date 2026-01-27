#pragma once

enum class MangoHudLevel { NO_DISPLAY = 0, FPS_ONLY = 1, HORIZONTAL = 2, EXTENDED = 3, DETAILED = 4 };

inline MangoHudLevel getEffective(MangoHudLevel level) {
	if (level == MangoHudLevel::DETAILED) {
		return MangoHudLevel::EXTENDED;
	}
	return level;
}