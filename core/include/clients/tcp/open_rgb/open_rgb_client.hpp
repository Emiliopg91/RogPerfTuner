#pragma once

#include "RccCommons.hpp"

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

    OpenRgbClient();

    void loadCompatibleDevices();
    void startOpenRgbProcess();
    void runner();
};