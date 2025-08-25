#include "RccCommons.hpp"

#include "../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../include/services/open_rgb_service.hpp"

OpenRgbService::OpenRgbService()
{
    logger.info("Initializing OpenRgbService");
    logger.add_tab();

    auto compatibles = OpenRgbClient::getInstance().getCompatibleDevices();
    for (auto d : compatibles)
    {
        compatibleDeviceNames[d.id_vendor + ":" + d.id_product] = d.name;
    }

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
