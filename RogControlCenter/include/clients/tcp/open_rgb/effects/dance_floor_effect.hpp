#pragma once

#include <random>
#include <vector>

#include "abstract/abstract_effect.hpp"

class DanceFloorEffect : public AbstractEffect {
  private:
	std::mt19937 _rng;

	Color _get_random_color() {
		std::uniform_int_distribution<int> hue_dist(0, 359);
		std::uniform_int_distribution<int> sat_dist(80, 100);
		std::uniform_int_distribution<int> val_dist(100, 100);	// valor fijo al 100%

		int h = hue_dist(_rng);
		int s = sat_dist(_rng);
		int v = val_dist(_rng);

		return Color::fromHsv(h, s / 100.0, v / 100.0);
	}

	std::vector<Color> _get_random_colors(size_t length) {
		std::vector<Color> colors;
		colors.reserve(length);
		for (size_t i = 0; i < length; ++i) {
			colors.push_back(_get_random_color());
		}
		return colors;
	}

  public:
	static DanceFloorEffect& getInstance(Client& client) {
		static DanceFloorEffect instance{client};
		return instance;
	}
	DanceFloorEffect(Client& client) : AbstractEffect(client, "Dance floor") {
		std::random_device rd;
		_rng = std::mt19937(rd());
	}

  protected:
	void apply_effect(const DeviceList& devices) {
		while (_is_running) {
			for (auto& dev : devices) {
				if (dev.enabled) {
					_set_colors(dev, _get_random_colors(dev.leds.size()));
				}
			}
			_sleep(0.5);
		}
	}
};
