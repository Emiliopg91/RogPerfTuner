#pragma once

#include <nlohmann/json.hpp>

#include "../../events/event_bus.hpp"
#include "../../services/hardware_service.hpp"
#include "../../services/open_rgb_service.hpp"
#include "../../services/performance_service.hpp"
#include "../../services/steam_service.hpp"
#include "httplib.h"

class HttpServer : public Singleton<HttpServer>, Loggable {
  private:
	httplib::Server svr;
	std::thread runner;
	std::atomic<bool> started{false};

	EventBus& eventBus					   = EventBus::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();

	HttpServer();
	friend class Singleton<HttpServer>;

  public:
	~HttpServer();

	void stop();
};
