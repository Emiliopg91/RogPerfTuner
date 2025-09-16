#include "../../include/services/open_rgb_service.hpp"

#include "../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../include/configuration/configuration.hpp"
#include "../../include/events/event_bus.hpp"
#include "../../include/models/hardware/usb_identifier.hpp"
#include "../../include/utils/time_utils.hpp"

OpenRgbService::OpenRgbService() {
	logger.info("Initializing OpenRgbService");
	Logger::add_tab();

	openRgbClient.initialize();

	restoreAura();

	eventBus.onBattery([this](bool onBat) {
		auto brightness = onBat ? RgbBrightness::Enum::OFF : this->brightness;
		openRgbClient.applyEffect(effect, brightness);
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
	effect	   = configuration.getConfiguration().open_rgb.last_effect.value_or("Static");
	brightness = configuration.getConfiguration().open_rgb.brightness;

	eventBus.emitRgbEffect(effect);
	eventBus.emitRgbBrightness(brightness);

	applyAura();
}

std::string OpenRgbService::getDeviceName(const UsbIdentifier& identifier) {
	for (auto dev : compatibleDevices) {
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
		applyAura(temporal);
		eventBus.emitRgbEffect(newEffect);
	}
}

void OpenRgbService::reload() {
	auto t0 = TimeUtils::now();
	logger.info("Reloading OpenRGB server");
	Logger::add_tab();
	openRgbClient.stop();
	openRgbClient.start();
	applyAura();
	auto t1 = TimeUtils::now();
	Logger::rem_tab();
	logger.info("Reloaded after {} ms", TimeUtils::getTimeDiff(t0, t1));
}

void OpenRgbService::applyAura(const bool& temporal) {
	logger.info("Applying aura settings");
	Logger::add_tab();
	auto t0 = TimeUtils::now();
	openRgbClient.applyEffect(effect, brightness);

	if (!temporal) {
		configuration.getConfiguration().open_rgb.brightness  = brightness;
		configuration.getConfiguration().open_rgb.last_effect = effect;
		configuration.saveConfig();
	}

	auto t1 = TimeUtils::now();
	Logger::rem_tab();
	logger.info("Aura applied after {} ms", TimeUtils::getTimeDiff(t0, t1));
}

void OpenRgbService::disableDevice(const UsbIdentifier& identifier) {
	logger.info("Disabling {}", getDeviceName(identifier));
	openRgbClient.disableDevice(getDeviceName(identifier));
}

RgbBrightness OpenRgbService::increaseBrightness() {
	auto next = brightness.getNextBrightness();
	setBrightness(next);
	return next;
}

RgbBrightness OpenRgbService::decreaseBrightness() {
	auto next = brightness.getPreviousBrightness();
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