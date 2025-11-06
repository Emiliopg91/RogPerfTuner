#pragma once

#include "../../models/hardware/battery_charge_threshold.hpp"
#include "../../models/others/singleton.hpp"
#include "abstract/abstract_file_client.hpp"

class BatteryChargeLimitClient : public AbstractFileClient, public Singleton<BatteryChargeLimitClient> {
  private:
	BatteryChargeLimitClient();
	friend class Singleton<BatteryChargeLimitClient>;

  public:
	void setChargeLimit(BatteryThreshold value);
};