#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"
#include "models/performance/power_profile.hpp"

class PowerProfileClient : public AbstractFileClient, public Singleton<PowerProfileClient> {
  public:
	PowerProfile getPowerProfile();

	void setPowerProfile(const PowerProfile& val);

  private:
	friend class Singleton<PowerProfileClient>;
	PowerProfileClient();
};