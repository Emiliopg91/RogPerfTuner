#include <mutex>

#include "RccCommons.hpp"

class OpenRgbService
{

public:
    static OpenRgbService &getInstance()
    {
        static OpenRgbService instance;
        return instance;
    }

    std::string getDeviceName(UsbIdentifier);

private:
    OpenRgbService();

    std::map<std::string, std::string> compatibleDeviceNames;
    Logger logger{"OpenRgbService"};
};