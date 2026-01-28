#include "clients/tcp/open_rgb/effects/static_effect.hpp"

void StaticEffect::apply_effect(const DeviceList& devices) {
	for (auto& dev : devices) {
		std::vector<Color> colors(dev.leds.size(), *_color);
		_set_colors(dev, colors);
	}
	while (_is_running) {
		_sleep(1.0);
	}
}

StaticEffect::StaticEffect(Client& client) : AbstractEffect(client, "Static", Color::Red.toHex()) {
}