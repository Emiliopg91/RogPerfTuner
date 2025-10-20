#pragma once

#include "../events/event_bus.hpp"
#include "../services/hardware_service.hpp"
#include "../services/open_rgb_service.hpp"
#include "../services/performance_service.hpp"
#include "../services/steam_service.hpp"

class SocketServer : public Singleton<SocketServer>, Loggable {
  private:
	int server_fd{-1};

	std::thread runner;
	std::atomic<bool> started{false};

	EventBus& eventBus					   = EventBus::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();

	void run();

	void handleClient(int client_fd);

	void handleRequest(const int& clientFd, const CommunicationMessage& req);
	void handleEvent(const CommunicationMessage& req);

	SocketServer();
	friend class Singleton<SocketServer>;

  public:
	~SocketServer();

	void stop();
};
