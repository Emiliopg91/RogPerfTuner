#pragma once

#include <string>

#include "../../models/performance/cpu_governor.hpp"
#include "./abstract/abstract_cmd_client.hpp"

class CpuPowerClient : public AbstractCmdClient {
  private:
	CpuPowerClient() : AbstractCmdClient("cpupower", "CpuPowerClient") {
	}

  public:
	static CpuPowerClient& getInstance() {
		static CpuPowerClient instance;
		return instance;
	}

	void setGovernor(const CpuGovernor& governor) {
		run_command("frequency-set -g " + governor.toString(), true, true);
	}
};