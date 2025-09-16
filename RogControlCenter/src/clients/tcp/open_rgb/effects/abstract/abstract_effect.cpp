#include "../../../../../../include/clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"

#include <mutex>
#include <thread>

#include "../../../../../../include/models/hardware/rgb_brightness.hpp"
#include "../../../../../../include/utils/string_utils.hpp"
#include "../../../../../../include/utils/time_utils.hpp"

double AbstractEffect::brightnessMap(const RgbBrightness& b) {
	if (b == RgbBrightness::Enum::LOW) {
		return 0.25;
	}
	if (b == RgbBrightness::Enum::MEDIUM) {
		return 0.5;
	}
	if (b == RgbBrightness::Enum::HIGH) {
		return 0.75;
	}

	return 1;
}

void AbstractEffect::start(const DeviceList& devices, const RgbBrightness& brightness) {
	if (_is_running) {
		stop();
	}

	if (brightness == RgbBrightness::Enum::OFF) {
		_logger.info("Turning off RGB");
		for (auto& dev : devices) {
			_client.setDeviceColor(dev, Color::Black);
		}
		return;
	}

	_logger.info("Starting effect '{}' with {} brightness", getName(), StringUtils::toLowerCase(brightness.toName()));
	_brightness = brightnessMap(brightness);
	_is_running = true;

	_thread = std::thread([this, &devices] {
		this->_thread_main(devices);
	});
}

void AbstractEffect::stop() {
	if (_is_running) {
		_logger.info("Stopping effect");
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
	_logger.info("Effect finished");
}

AbstractEffect::AbstractEffect(Client& client, const std::string& name) : _name(name), _client(client) {
}

const std::string AbstractEffect::getName() {
	return _name;
}