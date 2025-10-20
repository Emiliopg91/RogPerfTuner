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
	/**
	 * @brief Sets the CPU frequency scaling governor.
	 *
	 * This method configures the CPU to use the specified frequency scaling governor,
	 * which determines how the CPU frequency is managed (e.g., performance, powersave).
	 *
	 * @param governor The desired CPU governor to set.
	 */
	void setGovernor(const CpuGovernor& governor);

	friend class Singleton<CpuPowerClient>;
};