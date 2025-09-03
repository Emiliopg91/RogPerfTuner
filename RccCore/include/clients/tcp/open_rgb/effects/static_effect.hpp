#pragma once

#include "abstract/abstract_effect.hpp"

class StaticEffect : public AbstractEffect {
   public:
	static StaticEffect& getInstance(Client& client) {
		static StaticEffect instance{client};
		return instance;
	}

	void apply_effect(const DeviceList& devices) {
		for (auto& dev : devices) {
			std::vector<Color> colors(dev.leds.size(), Color::Red);
			_set_colors(dev, colors);
		}
		while (_is_running) {
			_sleep(1.0);
		}
	}

   private:
	StaticEffect(Client& client) : AbstractEffect(client, "Static") {
	}
};