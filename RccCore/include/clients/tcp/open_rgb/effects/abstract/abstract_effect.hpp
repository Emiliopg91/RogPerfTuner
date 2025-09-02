#pragma once

#include <string>

#include "RccCommons.hpp"
#include "OpenRGB/Client.hpp"
#include "../../../../../models/hardware/rgb_brightness.hpp"

using orgb::Client;
using orgb::Color;
using orgb::ConnectStatus;
using orgb::Device;
using orgb::DeviceList;
using orgb::DeviceListResult;
using orgb::DeviceType;
using orgb::enumString;
using orgb::Mode;
using orgb::RequestStatus;

class AbstractEffect
{
protected:
    bool _is_running = false;
    double _brightness = 0;
    std::string _name;
    Logger _logger;
    std::mutex _mutex;
    std::thread _thread;
    Client &_client;

    static double brightnessMap(RgbBrightness b);

public:
    AbstractEffect(Client &client, const std::string &name)
        : _name(name), _client(client)
    {
    }

    virtual ~AbstractEffect()
    {
        stop();
    }

    void start(DeviceList &devices, RgbBrightness brightness);

    void stop();

    std::string getName()
    {
        return _name;
    }

protected:
    void _sleep(double seconds);

    void _set_colors(Device &dev, const std::vector<orgb::Color> &colors);
    virtual void apply_effect(DeviceList &devices) = 0;

private:
    void _thread_main(DeviceList &devices)
    {
        apply_effect(devices);
        _logger.info("Effect finished");
    }
};