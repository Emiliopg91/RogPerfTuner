#pragma once

#include "RccCommons.hpp"
#include "OpenRGB/Client.hpp"

#include "../../../../include/models/usb_identifier.hpp"

#include "effects/abstract/abstract_effect.hpp"

class OpenRgbClient
{
public:
    static OpenRgbClient &getInstance()
    {
        static OpenRgbClient instance;
        return instance;
    }

    std::vector<UsbIdentifier> getCompatibleDevices();
    std::string getDeviceName(UsbIdentifier);
    std::vector<std::string> getAvailableEffects();
    void applyEffect(std::string effect, RgbBrightness brightness);
    void start();
    void stop();
    void disableDevice(std::string);

private:
    std::vector<UsbIdentifier> compatibleDevices;
    std::map<std::string, std::string> compatibleDeviceNames;
    std::thread runnerThread;
    Logger logger{"OpenRgbClient"};
    int port = 0;
    pid_t pid = 0;
    orgb::Client client{"RogControlCenters"};
    orgb::DeviceList detectedDevices;
    std::vector<std::unique_ptr<AbstractEffect>> availableEffects;

    OpenRgbClient();

    void loadCompatibleDevices();
    void startOpenRgbProcess();
    void startOpenRgbClient();
    void stopOpenRgbProcess();
    void getAvailableDevices();
    void runner();
};