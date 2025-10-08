#include "../../../../../include/clients/tcp/open_rgb/effects/dance_floor_effect.hpp"

Color DanceFloorEffect::_get_random_color() {
	std::uniform_int_distribution<int> hue_dist(0, 359);
	std::uniform_int_distribution<int> sat_dist(80, 100);
	std::uniform_int_distribution<int> val_dist(100, 100);	// valor fijo al 100%

	int h = hue_dist(_rng);
	int s = sat_dist(_rng);
	int v = val_dist(_rng);

	return Color::fromHsv(h, s / 100.0, v / 100.0);
}

std::vector<Color> DanceFloorEffect::_get_random_colors(size_t length) {
	std::vector<Color> colors;
	colors.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		colors.push_back(_get_random_color());
	}
	return colors;
}

DanceFloorEffect::DanceFloorEffect(Client& client) : AbstractEffect(client, "Dance floor") {
	std::random_device rd;
	_rng = std::mt19937(rd());
}

void DanceFloorEffect::apply_effect(const DeviceList& devices) {
	while (_is_running) {
		for (auto& dev : devices) {
			if (dev.enabled) {
				_set_colors(dev, _get_random_colors(dev.leds.size()));
			}
		}
		_sleep(0.5);
	}
}