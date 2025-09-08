#include "../../include/services/open_rgb_service.hpp"

#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../include/configuration/configuration.hpp"
#include "../../include/models/hardware/usb_identifier.hpp"
#include "../../include/translator/translator.hpp"
#include "RccCommons.hpp"

OpenRgbService::OpenRgbService() {
	logger.info("Initializing OpenRgbService");
	logger.add_tab();

	OpenRgbClient::getInstance();

	restoreAura();

	eventBus.on_without_data(Events::HARDWARE_SERVICE_ON_BATTERY, [this]() {
		auto brightness = UPowerClient::getInstance().isOnBattery() ? RgbBrightness::Enum::OFF : this->brightness;
		OpenRgbClient::getInstance().applyEffect(effect, brightness);
	});

	eventBus.on_without_data(Events::HARDWARE_SERVICE_USB_ADDED_REMOVED, [this]() { reload(); });

	logger.rem_tab();
}

void OpenRgbService::restoreAura() {
	effect	   = configuration.getConfiguration().open_rgb.last_effect.value_or("Static");
	brightness = configuration.getConfiguration().open_rgb.brightness;

	applyAura();
}

std::string OpenRgbService::getDeviceName(const UsbIdentifier& identifier) {
	std::string key = std::string(identifier.id_vendor) + ":" + std::string(identifier.id_product);
	auto it			= compatibleDeviceNameMap.find(key);
	if (it != compatibleDeviceNameMap.end()) {
		return it->second;
	}

	return "";
}

std::vector<std::string> OpenRgbService::getAvailableEffects() {
	auto all = OpenRgbClient::getInstance().getAvailableEffects();
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
		eventBus.emit_event(Events::ORGB_SERVICE_ON_BRIGHTNESS, {newBrightness});
	}
}

void OpenRgbService::setEffect(const std::string& newEffect, const bool& temporal) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (effect != newEffect) {
		effect = newEffect;
		applyAura(temporal);
		eventBus.emit_event(Events::ORGB_SERVICE_ON_EFFECT, {newEffect});
	}
}

void OpenRgbService::reload() {
	auto t0 = std::chrono::high_resolution_clock::now();
	logger.info("Reloading OpenRGB server");
	logger.add_tab();
	OpenRgbClient::getInstance().stop();
	OpenRgbClient::getInstance().start();
	applyAura();
	auto t1 = std::chrono::high_resolution_clock::now();
	logger.rem_tab();
	logger.info("Reloaded after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");
}

void OpenRgbService::applyAura(const bool& temporal) {
	logger.info("Applying aura settings");
	logger.add_tab();
	auto t0 = std::chrono::high_resolution_clock::now();
	OpenRgbClient::getInstance().applyEffect(effect, brightness);

	if (!temporal) {
		configuration.getConfiguration().open_rgb.brightness  = brightness;
		configuration.getConfiguration().open_rgb.last_effect = effect;
		configuration.saveConfig();
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	logger.rem_tab();
	logger.info("Aura applied after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");
}

void OpenRgbService::disableDevice(const UsbIdentifier& identifier) {
	OpenRgbClient::getInstance().disableDevice(getDeviceName(identifier));
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
	if (it == list.end())
		it = list.begin();
	auto next = *it;
	setEffect(next);
	return next;
}