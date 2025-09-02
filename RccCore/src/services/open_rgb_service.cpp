#include "RccCommons.hpp"

#include "../../include/configuration/configuration.hpp"
#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../include/models/hardware/usb_identifier.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/translator/translator.hpp"
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

    restoreAura();

    EventBus::getInstance().on_without_data(Events::HARDWARE_SERVICE_ON_BATTERY, [this]()
                                            { 
                                auto brightness = UPowerClient::getInstance().isOnBattery() ? RgbBrightness::Enum::OFF : this->brightness; 
                                OpenRgbClient::getInstance().applyEffect(effect, brightness); });

    EventBus::getInstance().on_without_data(Events::HARDWARE_SERVICE_USB_ADDED_REMOVED, [this]()
                                            { reload(); });

    logger.rem_tab();
}

void OpenRgbService::restoreAura()
{
    effect = Configuration::getInstance().getConfiguration().open_rgb.last_effect.value_or("Static");
    brightness = Configuration::getInstance().getConfiguration().open_rgb.brightness;

    applyAura();
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
    std::lock_guard<std::mutex> lock(actionMutex);
    if (brightness != newBrightness)
    {
        brightness = newBrightness;
        applyAura();
        EventBus::getInstance().emit_event(Events::ORGB_SERVICE_ON_BRIGHTNESS, {newBrightness});
    }
}

void OpenRgbService::setEffect(std::string newEffect, bool temporal)
{
    std::lock_guard<std::mutex> lock(actionMutex);
    if (effect != newEffect)
    {
        effect = newEffect;
        applyAura(temporal);
        EventBus::getInstance().emit_event(Events::ORGB_SERVICE_ON_EFFECT, {newEffect});
    }
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

void OpenRgbService::applyAura(bool temporal)
{
    logger.info("Applying aura settings");
    logger.add_tab();
    auto t0 = std::chrono::high_resolution_clock::now();
    OpenRgbClient::getInstance().applyEffect(effect, brightness);

    if (!temporal)
    {
        Configuration::getInstance().getConfiguration().open_rgb.brightness = brightness;
        Configuration::getInstance().getConfiguration().open_rgb.last_effect = effect;
        Configuration::getInstance().saveConfig();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    logger.rem_tab();
    logger.info("Aura applied after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");
}

void OpenRgbService::disableDevice(UsbIdentifier identifier)
{
    OpenRgbClient::getInstance().disableDevice(getDeviceName(identifier));
}

RgbBrightness OpenRgbService::increaseBrightness()
{
    auto next = brightness.getNextBrightness();
    OpenRgbService::getInstance().setBrightness(next);
    return next;
}

RgbBrightness OpenRgbService::decreaseBrightness()
{
    auto next = brightness.getPreviousBrightness();
    OpenRgbService::getInstance().setBrightness(next);
    return next;
}

std::string OpenRgbService::nextEffect()
{
    auto list = getAvailableEffects();
    auto it = std::find(list.begin(), list.end(), effect);
    ++it;
    if (it == list.end())
        it = list.begin();
    auto next = *it;
    OpenRgbService::getInstance().setEffect(next);
    return next;
}