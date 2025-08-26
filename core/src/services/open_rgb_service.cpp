#include "RccCommons.hpp"

#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/utils/events.hpp"

OpenRgbService::OpenRgbService()
{
    logger.info("Initializing OpenRgbService");
    logger.add_tab();

    auto compatibles = OpenRgbClient::getInstance().getCompatibleDevices();
    for (auto d : compatibles)
    {
        compatibleDeviceNames[d.id_vendor + ":" + d.id_product] = d.name;
    }

    effect = Configuration::getInstance().getConfiguration().open_rgb.last_effect.value_or("Static");
    brightness = Configuration::getInstance().getConfiguration().open_rgb.brightness;

    applyAura();

    EventBus::getInstance().on(Events::HARDWARE_SERVICE_ON_BATTERY, [this]()
                               { auto brightness = UPowerClient::getInstance().isOnBattery() ? RgbBrightness::Enum::OFF : this->brightness; 
                                OpenRgbClient::getInstance().applyEffect(effect, brightness); });

    EventBus::getInstance().on(Events::HARDWARE_SERVICE_USB_ADDED_REMOVED, [this]()
                               { reload(); });

    logger.rem_tab();
}

std::string OpenRgbService::getDeviceName(UsbIdentifier identifier)
{
    std::string key = identifier.id_vendor + ":" + identifier.id_product;
    auto it = compatibleDeviceNames.find(key);
    if (it != compatibleDeviceNames.end())
    {
        return it->second;
    }

    return "";
}

std::vector<std::string> OpenRgbService::getAvailableEffects()
{
    auto all = OpenRgbClient::getInstance().getAvailableEffects();
    std::sort(all.begin(), all.end());
    return all;
}

std::string OpenRgbService::getCurrentEffect()
{
    return effect;
}

RgbBrightness OpenRgbService::getCurrentBrightness()
{
    return brightness;
}

void OpenRgbService::setBrightness(RgbBrightness newBrightness)
{
    brightness = newBrightness;
    applyAura();
}

void OpenRgbService::setEffect(std::string newEffect)
{
    effect = newEffect;
    applyAura();
}

void OpenRgbService::reload()
{
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

void OpenRgbService::applyAura()
{
    logger.info("Applying aura settings");
    logger.add_tab();
    auto t0 = std::chrono::high_resolution_clock::now();
    OpenRgbClient::getInstance().applyEffect(effect, brightness);

    Configuration::getInstance().getConfiguration().open_rgb.brightness = brightness;
    Configuration::getInstance().getConfiguration().open_rgb.last_effect = effect;
    Configuration::getInstance().saveConfig();

    auto t1 = std::chrono::high_resolution_clock::now();
    logger.rem_tab();
    logger.info("Aura applied after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");
}