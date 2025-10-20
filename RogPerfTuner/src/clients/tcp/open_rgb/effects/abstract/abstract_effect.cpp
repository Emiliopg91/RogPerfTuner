#include "../../../../../../include/clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"

#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "../../../../../../include/models/hardware/rgb_brightness.hpp"
#include "../../../../../../include/utils/string_utils.hpp"
#include "../../../../../../include/utils/time_utils.hpp"

void AbstractEffect::start(const DeviceList& devices, const RgbBrightness& brightness) {
	if (_is_running) {
		stop();
	}

	if (brightness == RgbBrightness::Enum::OFF) {
		logger.info("Turning off RGB");
		for (auto& dev : devices) {
			_client.setDeviceColor(dev, Color::Black);
		}
		return;
	}

	if (!_color.has_value()) {
		logger.info("Starting effect '{}' with {} brightness", getName(), StringUtils::toLowerCase(brightness.toName()));
	} else {
		logger.info("Starting effect '{}' with {} brightness and color {}", getName(), StringUtils::toLowerCase(brightness.toName()),
					StringUtils::toUpperCase(_color.value().toHex()));
	}
	_brightness = brightness.toInt() / 100.0;
	_is_running = true;

	_thread = std::thread([this, &devices] {
		this->_thread_main(devices);
	});
}

void AbstractEffect::stop() {
	if (_is_running) {
		logger.info("Stopping effect");
		_is_running = false;
		if (_thread.joinable()) {
			_thread.join();
		}
	}
}

void AbstractEffect::_sleep(const double& seconds) {
	double remaining = seconds * 1000;
	while (_is_running && remaining > 0) {
		double nap = std::min(100.0, remaining);
		TimeUtils::sleep(nap);
		remaining -= nap;
	}
}

void AbstractEffect::_set_colors(Device& dev, const std::vector<orgb::Color>& colors) {
	if (dev.enabled && _is_running) {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_is_running) {
			std::vector<orgb::Color> dimmed_colors;
			for (const auto& c : colors) {
				dimmed_colors.push_back(c * _brightness);
			}
			_client.setDeviceColors(dev, dimmed_colors);
		}
	}
}

void AbstractEffect::_thread_main(const DeviceList& devices) {
	apply_effect(devices);
	logger.info("Effect finished");
}

AbstractEffect::AbstractEffect(Client& client, const std::string& name, const std::optional<std::string>& color)
	: Loggable(name), _name(name), _client(client) {
	auto parts = StringUtils::split(name, ' ');
	for (size_t i = 0; i < parts.size(); i++) {
		parts[i] = StringUtils::capitalize(parts[i]);
	}
	logger = Logger{StringUtils::join(parts, "") + "Effect"};
	if (color.has_value()) {
		_color = Color::fromRgb(*color);
	}
}

const std::string AbstractEffect::getName() {
	return _name;
}

bool AbstractEffect::supportsColor() {
	return _color.has_value();
}

void AbstractEffect::setColor(std::string color) {
	if (supportsColor()) {
		_color = Color::fromRgb(color);
	}
}

std::optional<std::string> AbstractEffect::getColor() {
	std::optional<std::string> res = std::nullopt;
	if (_color.has_value()) {
		res = _color->toHex();
	}
	return res;
}