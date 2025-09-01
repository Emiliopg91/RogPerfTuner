#pragma once

#include <mutex>

#include "RccCommons.hpp"
#include "../models/rgb_brightness.hpp"
#include "../models/usb_identifier.hpp"

class OpenRgbService
{

public:
    static OpenRgbService &getInstance()
    {
        static OpenRgbService instance;
        return instance;
    }

    std::string getDeviceName(UsbIdentifier);
    std::vector<std::string> getAvailableEffects();
    std::string getCurrentEffect();
    RgbBrightness getCurrentBrightness();

    void setBrightness(RgbBrightness newBrightness);
    void setEffect(std::string newEffect, bool temporal = false);
    void restoreAura();
    void disableDevice(UsbIdentifier);

    RgbBrightness increaseBrightness();
    RgbBrightness decreaseBrightness();
    std::string nextEffect();

private:
    OpenRgbService();

    std::map<std::string, std::string> compatibleDeviceNames;
    Logger logger{"OpenRgbService"};
    RgbBrightness brightness = RgbBrightness::Enum::MAX;
    std::string effect;

    void applyAura(bool temporal = false);
    void reload();
};