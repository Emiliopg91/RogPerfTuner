#pragma once

#include <mutex>
#include <string>

#include "../../../../../logger/logger.hpp"
#include "../../../../../models/hardware/rgb_brightness.hpp"
#include "OpenRGB/Client.hpp"

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

class AbstractEffect {
  protected:
	bool _is_running   = false;
	double _brightness = 0;
	std::string _name;
	Logger _logger;
	std::mutex _mutex;
	std::thread _thread;
	Client& _client;

  public:
	AbstractEffect(Client& client, const std::string& name);

	virtual ~AbstractEffect() {
		stop();
	}

	const std::string getName();

	void start(const DeviceList& devices, const RgbBrightness& brightness);

	void stop();

  protected:
	void _sleep(const double& seconds);

	void _set_colors(Device& dev, const std::vector<orgb::Color>& colors);
	virtual void apply_effect(const DeviceList& devices) = 0;

  private:
	void _thread_main(const DeviceList& devices);
};