#include "clients/file/battery_charge_limit_client.hpp"

#include "framework/utils/enum_utils.hpp"

BatteryChargeLimitClient::BatteryChargeLimitClient() : AbstractFileClient(BAT_LIMIT_FILE, "BatteryChargeLimitClient", true, false) {
}

void BatteryChargeLimitClient::setChargeLimit(BatteryThreshold value) {
	write(std::to_string(toInt(value)));
}