#pragma once

#include "RccCommons.hpp"
#include "OpenRGB/Client.hpp"

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

private:
    std::vector<UsbIdentifier> compatibleDevices;
    std::map<std::string, std::string> compatibleDeviceNames;
    std::thread runnerThread;
    Logger logger{"OpenRgbClient"};
    int port = 0;
    orgb::Client client{"RogControlCenters"};

    OpenRgbClient();

    void loadCompatibleDevices();
    void startOpenRgbProcess();
    void startOpenRgbClient();
    void getAvailableDevices();
    void runner();
};