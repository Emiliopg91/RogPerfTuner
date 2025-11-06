#include "../../../include/clients/file/battery_charge_limit_client.hpp"

#include <string>

BatteryChargeLimitClient::BatteryChargeLimitClient()
	: AbstractFileClient("/sys/class/power_supply/BAT0/charge_control_end_threshold", "BatteryChargeLimitClient", true, false) {
}

void BatteryChargeLimitClient::setChargeLimit(BatteryThreshold value) {
	write(std::to_string(value.toInt()));
}