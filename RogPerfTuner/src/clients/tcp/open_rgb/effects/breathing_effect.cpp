#include "../../../../../include/clients/tcp/open_rgb/effects/breathing_effect.hpp"

void BreathingEffect::apply_effect(const DeviceList& devices) {
	int offset = 0;
	while (_is_running) {
		Color new_color = *_color * _sin_steps[offset];

		for (auto& dev : devices) {
			if (dev.enabled) {
				std::vector<Color> colors(dev.leds.size(), new_color);
				_set_colors(dev, colors);
			}
		}

		offset = (offset + 1) % _step_count;

		if (offset > 0) {
			_sleep(_frequency);
		} else {
			_sleep(_pause_time);
		}
	}
}

BreathingEffect::BreathingEffect(Client& client)
	: AbstractEffect(client, "Breathing", Color::Red.toHex()), _total_time(4.0), _pause_time(1.0), _frequency(0.05) {
	_step_count = static_cast<int>((_total_time - _pause_time) / _frequency);
	_sin_steps.reserve(_step_count);
	for (int offset = 0; offset < _step_count; ++offset) {
		_sin_steps.push_back(std::sin(M_PI * (static_cast<double>(offset) / (_step_count - 1))));
	}
}