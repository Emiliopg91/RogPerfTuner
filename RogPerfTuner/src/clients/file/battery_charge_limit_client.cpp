#include "../../../include/clients/file/battery_charge_limit_client.hpp"

#include <string>

#include "../../../include/utils/enum_utils.hpp"

BatteryChargeLimitClient::BatteryChargeLimitClient()
	: AbstractFileClient("/sys/class/power_supply/BAT0/charge_control_end_threshold", "BatteryChargeLimitClient", true, false) {
}

void BatteryChargeLimitClient::setChargeLimit(BatteryThreshold value) {
	write(std::to_string(toInt(value)));
}