#include "clients/file/battery_status_client.hpp"

#include <any>
#include <string>

#include "framework/utils/string_utils.hpp"

BatteryStatusClient::BatteryStatusClient() : AbstractFileClient("/sys/class/power_supply/BAT0/status", "BatteryChargeLimitClient", true, false) {
	lastState	  = isOnBattery();
	running		  = true;
	pollingThread = std::thread(&BatteryStatusClient::monitorLoop, this, 1000);
}

BatteryStatusClient::~BatteryStatusClient() {
	running = false;
	if ((*pollingThread).joinable()) {
		(*pollingThread).join();
	}
}

bool BatteryStatusClient::isOnBattery() {
	return StringUtils::trim(read()) == "Discharging";
}

void BatteryStatusClient::onBatteryChange(std::function<void(std::any)>&& callback) {
	this->eventBus.onBatteryStatus([cb = std::move(callback)](bool onBattery) {
		cb(std::any_cast<bool>(onBattery));
	});
}

void BatteryStatusClient::monitorLoop(int intervalMs) {
	while (running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));

		bool currentState = isOnBattery();
		if (currentState != lastState) {
			lastState = currentState;
			eventBus.emitBatteryStatus(currentState);
		}
	}
}