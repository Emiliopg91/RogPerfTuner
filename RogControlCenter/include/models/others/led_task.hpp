#pragma once

#include <cstddef>

#include "OpenRGB/Color.hpp"

struct LedTask {
	size_t index;
	orgb::Color color;
};