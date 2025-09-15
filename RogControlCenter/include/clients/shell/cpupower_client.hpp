#pragma once

#include <string>

#include "../../models/others/singleton.hpp"
#include "../../models/performance/cpu_governor.hpp"
#include "./abstract/abstract_cmd_client.hpp"

class CpuPowerClient : public AbstractCmdClient, public Singleton<CpuPowerClient> {
  private:
	CpuPowerClient() : AbstractCmdClient("cpupower", "CpuPowerClient") {
	}

  public:
	void setGovernor(const CpuGovernor& governor);
	friend class Singleton<CpuPowerClient>;
};