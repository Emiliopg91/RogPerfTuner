#include "services/open_rgb_service.hpp"

#include <optional>
#include <string>

#include "clients/tcp/open_rgb/open_rgb_client.hpp"
#include "models/hardware/usb_identifier.hpp"
#include "time_utils.hpp"
#include "utils/configuration_wrapper.hpp"
#include "utils/event_bus_wrapper.hpp"

OpenRgbService::OpenRgbService() : Loggable("OpenRgbService") {
	logger->info("Initializing OpenRgbService");
	Logger::add_tab();

	openRgbClient.initialize();

	restoreAura();

	eventBus.onBattery([this](bool onBat) {
		auto brightness = onBat ? RgbBrightness::OFF : this->brightness;
		openRgbClient.applyEffect(effect, brightness, _color);
	});

	eventBus.onUsbAdded([this]() {
		reload();
	});

	eventBus.onUsbRemoved([this](std::vector<UsbIdentifier> rem) {
		for (auto dev : rem) {
			disableDevice(dev);
		}
	});

	Logger::rem_tab();
}

void OpenRgbService::restoreAura() {
	effect	   = configuration.getConfiguration().aura.last_effect.value_or("Static");
	brightness = configuration.getConfiguration().aura.brightness;
	_color	   = std::nullopt;

	auto it = configuration.getConfiguration().aura.config.find(effect);
	if (it != configuration.getConfiguration().aura.config.end()) {
		_color = it->second.color;
	}

	applyAura();

	if (!_color.has_value()) {
		auto c = openRgbClient.getColor();
		if (c.has_value()) {
			_color = c.value();
		}
	}

	eventBus.emitRgbEffect(effect);
	eventBus.emitRgbBrightness(brightness);
	eventBus.emitRgbColor(_color);
}

std::string OpenRgbService::getDeviceName(const UsbIdentifier& identifier) {
	for (auto dev : openRgbClient.getCompatibleDevices()) {
		if (identifier == dev) {
			return std::string(dev.name);
		}
	}

	return "";
}

std::vector<std::string> OpenRgbService::getAvailableEffects() {
	auto all = openRgbClient.getAvailableEffects();
	std::sort(all.begin(), all.end());
	return all;
}

std::string OpenRgbService::getCurrentEffect() {
	return effect;
}

RgbBrightness OpenRgbService::getCurrentBrightness() {
	return brightness;
}

std::optional<std::string> OpenRgbService::getColor() {
	return _color;
}

void OpenRgbService::setBrightness(const RgbBrightness& newBrightness) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (brightness != newBrightness) {
		brightness = newBrightness;
		applyAura();
		eventBus.emitRgbBrightness(newBrightness);
	}
}

void OpenRgbService::setEffect(const std::string& newEffect, const bool& temporal) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (effect != newEffect) {
		effect = newEffect;

		_color	= std::nullopt;
		auto it = configuration.getConfiguration().aura.config.find(effect);
		if (it != configuration.getConfiguration().aura.config.end()) {
			_color = it->second.color;
		}

		applyAura(temporal);

		if (!_color.has_value()) {
			_color = openRgbClient.getColor();
		}

		eventBus.emitRgbEffect(newEffect);
		eventBus.emitRgbColor(openRgbClient.getColor());
	}
}

void OpenRgbService::setColor(const std::string& color) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (_color.has_value()) {
		_color = color;
		applyAura();
		eventBus.emitRgbColor(openRgbClient.getColor());
	}
}

void OpenRgbService::reload() {
	auto t0 = TimeUtils::now();
	logger->info("Reloading OpenRGB server");
	Logger::add_tab();
	openRgbClient.stop();
	openRgbClient.start();
	applyAura();
	auto t1 = TimeUtils::now();
	Logger::rem_tab();
	logger->info("Reloaded after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));
}

void OpenRgbService::applyAura(const bool& temporal) {
	logger->info("Applying aura settings");
	Logger::add_tab();
	auto t0 = TimeUtils::now();
	openRgbClient.applyEffect(effect, brightness, _color);

	if (!temporal) {
		configuration.getConfiguration().aura.brightness  = brightness;
		configuration.getConfiguration().aura.last_effect = effect;

		if (_color.has_value()) {
			auto it = configuration.getConfiguration().aura.config.find(effect);
			if (it != configuration.getConfiguration().aura.config.end()) {
				it->second.color = _color.value();
			} else {
				configuration.getConfiguration().aura.config[effect] = EffectConfig(_color.value());
			}
		}

		configuration.saveConfig();
	}

	auto t1 = TimeUtils::now();
	Logger::rem_tab();
	logger->info("Aura applied after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));
}

void OpenRgbService::disableDevice(const UsbIdentifier& identifier) {
	logger->info("Disabling {}", getDeviceName(identifier));
	openRgbClient.disableDevice(getDeviceName(identifier));
}

RgbBrightness OpenRgbService::increaseBrightness() {
	auto next = getNextBrightness(brightness);
	setBrightness(next);
	return next;
}

RgbBrightness OpenRgbService::decreaseBrightness() {
	auto next = getPreviousBrightness(brightness);
	setBrightness(next);
	return next;
}

std::string OpenRgbService::nextEffect() {
	auto list = getAvailableEffects();
	auto it	  = std::find(list.begin(), list.end(), effect);
	++it;
	if (it == list.end()) {
		it = list.begin();
	}
	auto next = *it;
	setEffect(next);
	return next;
}