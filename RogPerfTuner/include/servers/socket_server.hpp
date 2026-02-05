#pragma once

#include "framework/clients/abstract/abstract_unix_socket_client.hpp"
#include "services/hardware_service.hpp"
#include "services/open_rgb_service.hpp"
#include "services/performance_service.hpp"
#include "services/steam_service.hpp"
#include "utils/event_bus_wrapper.hpp"

class SocketServer : public Singleton<SocketServer>, Loggable {
  private:
	int server_fd{-1};

	std::thread runner;
	std::atomic<bool> started{false};

	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();

	void run();

	void handleClient(int client_fd);

	void handleRequest(const int& clientFd, const UnixCommunicationMessage& req);
	void handleEvent(const UnixCommunicationMessage& req);

	SocketServer();
	friend class Singleton<SocketServer>;

  public:
	~SocketServer();

	void stop();
};
