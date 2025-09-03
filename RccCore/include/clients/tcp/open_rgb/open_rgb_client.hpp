#pragma once

#include "RccCommons.hpp"
#include "OpenRGB/Client.hpp"

#include "../../../../include/models/hardware/usb_identifier.hpp"

#include "effects/abstract/abstract_effect.hpp"

class OpenRgbClient
{
public:
    static OpenRgbClient &getInstance()
    {
        static OpenRgbClient instance;
        return instance;
    }

    const std::vector<UsbIdentifier> getCompatibleDevices();
    const std::vector<std::string> getAvailableEffects();
    void applyEffect(const std::string &effect, const RgbBrightness &brightness);
    void start();
    void stop();
    void disableDevice(const std::string &);

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