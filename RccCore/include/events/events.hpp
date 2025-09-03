#pragma once

#include <string>

class Events
{
public:
    inline const static std::string APPLICATION_STOP = "APPLICATION_STOP";

    inline const static std::string UDEV_CLIENT_DEVICE_EVENT = "UDEV_CLIENT_DEVICE_EVENT";

    inline const static std::string HARDWARE_SERVICE_USB_ADDED_REMOVED = "HARDWARE_SERVICE_USB_ADDED_REMOVED";
    inline const static std::string HARDWARE_SERVICE_ON_BATTERY = "HARDWARE_SERVICE_ON_BATTERY";
    inline const static std::string HARDWARE_SERVICE_THRESHOLD_CHANGED = "HARDWARE_SERVICE_THRESHOLD_CHANGED";

    inline const static std::string ORGB_SERVICE_ON_BRIGHTNESS = "ORGB_SERVICE_ON_BRIGHTNESS";
    inline const static std::string ORGB_SERVICE_ON_EFFECT = "ORGB_SERVICE_ON_EFFECT";

    inline const static std::string PROFILE_SERVICE_ON_PROFILE = "PROFILE_SERVICE_ON_PROFILE";

    inline const static std::string STEAM_SERVICE_GAME_EVENT = "STEAM_SERVICE_GAME_EVENT";
};