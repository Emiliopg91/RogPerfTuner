#include "../../../../../include/clients/tcp/open_rgb/effects/gaming.hpp"

const Color GamingEffect::MAIN_COLOR = Color::fromRgb("#000040");

const std::vector<std::pair<std::regex, Color>> GamingEffect::COLOR_MATCHING = {
	{std::regex("Key: [1|2|3|4]"), Color::fromRgb("#FFFF00")},
	{std::regex("Key: [W|A|S|D]"), Color::fromRgb("#FF0000")},
	{std::regex("Key: Media Volume [-+]|Key: Media Mute"), Color::fromRgb("#00FFFF")},
	{std::regex("Key: Fan|Key: ROG"), Color::fromRgb("#FF4000")},
};

GamingEffect::GamingEffect(Client& client) : AbstractEffect(client, "Gaming") {
}

void GamingEffect::apply_effect(const DeviceList& devices) {
	for (auto& dev : devices) {
		std::vector<Color> colors(dev.leds.size(), MAIN_COLOR);
		size_t offset = 0;

		if (dev.type == DeviceType::Keyboard) {
			for (auto& led : dev.leds) {
				for (auto& pair : COLOR_MATCHING) {
					if (std::regex_match(led.name, pair.first)) {
						colors[offset + led.idx] = pair.second;
						break;
					}
				}
			}
		}

		_set_colors(dev, colors);
	}
	while (_is_running) {
		_sleep(1.0);
	}
}
