#pragma once

#include <string>

class Events
{
public:
    inline const static std::string UDEV_CLIENT_DEVICE_EVENT = "UDEV_CLIENT_DEVICE_EVENT";
    inline const static std::string HARDWARE_SERVICE_USB_ADDED_REMOVED = "HARDWARE_SERVICE_USB_ADDED_REMOVED";
    inline const static std::string HARDWARE_SERVICE_ON_BATTERY = "HARDWARE_SERVICE_ON_BATTERY";
};