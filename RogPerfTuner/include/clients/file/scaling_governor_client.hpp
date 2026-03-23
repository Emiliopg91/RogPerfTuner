#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_glob_client.hpp"
#include "models/performance/cpu_governor.hpp"

class ScalingGovernorClient : public AbstractGlobClient, public Singleton<ScalingGovernorClient> {
  private:
	ScalingGovernorClient();

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

	friend class Singleton<ScalingGovernorClient>;
};