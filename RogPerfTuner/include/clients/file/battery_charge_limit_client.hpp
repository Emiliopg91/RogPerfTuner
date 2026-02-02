#pragma once

#include "abstracts/singleton.hpp"
#include "clients/abstract_file_client.hpp"
#include "models/hardware/battery_charge_threshold.hpp"

class BatteryChargeLimitClient : public AbstractFileClient, public Singleton<BatteryChargeLimitClient> {
  private:
	BatteryChargeLimitClient();
	friend class Singleton<BatteryChargeLimitClient>;

  public:
	void setChargeLimit(BatteryThreshold value);
};