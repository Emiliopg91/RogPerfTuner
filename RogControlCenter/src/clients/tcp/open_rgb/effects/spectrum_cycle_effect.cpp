#include "../../../../../include/clients/tcp/open_rgb/effects/spectrum_cycle_effect.hpp"

void SpectrumCycleEffect::apply_effect(const DeviceList& devices) {
	int offset = 0;
	while (_is_running) {
		for (auto& dev : devices) {
			if (dev.enabled) {
				std::vector<Color> colors(dev.leds.size(), Color::fromHsv(offset, 1, 1));
				_set_colors(dev, colors);
			}
		}

		offset = (offset + 1) % 360;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

SpectrumCycleEffect::SpectrumCycleEffect(Client& client) : AbstractEffect(client, "Spectrum cycle") {
}