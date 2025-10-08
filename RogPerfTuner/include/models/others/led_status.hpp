#pragma once

#include <cstdint>

constexpr uint32_t INVALID_LED = 0xFFFFFFFF;

struct LedStatus {
	uint32_t pos_idx = INVALID_LED;
	int max_val		 = 20;
	int cur_val		 = 0;

	LedStatus clone() const {
		return LedStatus{pos_idx, max_val, cur_val};
	}
};