#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"

class BatteryStatusClient : public AbstractFileClient, public Singleton<BatteryStatusClient> {
  private:
	BatteryStatusClient();
	friend class Singleton<BatteryStatusClient>;

  public:
	bool isCharging();
};