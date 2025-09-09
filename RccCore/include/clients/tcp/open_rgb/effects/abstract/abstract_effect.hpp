#pragma once

#include <mutex>
#include <string>

#include "../../../../../models/hardware/rgb_brightness.hpp"
#include "OpenRGB/Client.hpp"
#include "RccCommons.hpp"

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

	static double brightnessMap(const RgbBrightness& b);

  public:
	AbstractEffect(Client& client, const std::string& name) : _name(name), _client(client) {
	}

	virtual ~AbstractEffect() {
		stop();
	}

	void start(const DeviceList& devices, const RgbBrightness& brightness);

	void stop();

	const std::string getName() {
		return _name;
	}

  protected:
	void _sleep(const double& seconds);

	void _set_colors(Device& dev, const std::vector<orgb::Color>& colors);
	virtual void apply_effect(const DeviceList& devices) = 0;

  private:
	void _thread_main(const DeviceList& devices) {
		apply_effect(devices);
		_logger.info("Effect finished");
	}
};