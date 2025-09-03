#pragma once

#include <mutex>

#include "../models/hardware/rgb_brightness.hpp"
#include "../models/hardware/usb_identifier.hpp"
#include "RccCommons.hpp"

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

	std::map<std::string, std::string> compatibleDeviceNames;
	Logger logger{"OpenRgbService"};

	std::mutex actionMutex;

	RgbBrightness brightness = RgbBrightness::Enum::MAX;
	std::string effect;

	void applyAura(const bool& temporal = false);
	void reload();
};