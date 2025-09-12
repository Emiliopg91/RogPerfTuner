#pragma once

#include <mutex>

#include "../clients/dbus/linux/upower_client.hpp"
#include "../clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../configuration/configuration.hpp"
#include "../events/event_bus.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/hardware/usb_identifier.hpp"

class OpenRgbService {
  public:
	static OpenRgbService& getInstance() {
		static OpenRgbService instance;
		return instance;
	}

	std::string getDeviceName(const UsbIdentifier&);
	std::vector<std::string> getAvailableEffects();
	std::string getCurrentEffect();
	RgbBrightness getCurrentBrightness();

	void setBrightness(const RgbBrightness& newBrightness);
	void setEffect(const std::string& newEffect, const bool& temporal = false);
	void restoreAura();
	void disableDevice(const UsbIdentifier&);

	RgbBrightness increaseBrightness();
	RgbBrightness decreaseBrightness();
	std::string nextEffect();

  private:
	OpenRgbService();

	Logger logger{"OpenRgbService"};

	std::mutex actionMutex;

	RgbBrightness brightness = RgbBrightness::Enum::MAX;
	std::string effect;

	UPowerClient& uPowerClient	 = UPowerClient::getInstance();
	OpenRgbClient& openRgbClient = OpenRgbClient::getInstance();
	Configuration& configuration = Configuration::getInstance();
	EventBus& eventBus			 = EventBus::getInstance();

	void applyAura(const bool& temporal = false);
	void reload();
};