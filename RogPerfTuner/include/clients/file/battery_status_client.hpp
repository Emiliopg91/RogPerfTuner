#pragma once

#include <optional>
#include <thread>

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"
#include "framework/events/event_bus.hpp"
#include "utils/event_bus_wrapper.hpp"

class BatteryStatusClient : public AbstractFileClient, public Singleton<BatteryStatusClient> {
  private:
	BatteryStatusClient();
	void monitorLoop(int intervalMs);
	friend class Singleton<BatteryStatusClient>;
	std::optional<std::thread> pollingThread = std::nullopt;
	bool running							 = false;
	bool lastState;
	EventBusWrapper& eventBus = EventBusWrapper::getInstance();

  public:
	bool isOnBattery();
	void onBatteryChange(CallbackWithAnyParam&& callback);
	~BatteryStatusClient();
};