#pragma once

#include <string>
#include <vector>

#include "abstracts/clients/abstract_cmd_client.hpp"
#include "abstracts/singleton.hpp"
#include "models/hardware/gpu_info.hpp"

class SwitcherooCtlClient : AbstractCmdClient, public Singleton<SwitcherooCtlClient> {
  private:
	SwitcherooCtlClient() : AbstractCmdClient("switcherooctl", "SwitcherooCtlClient") {
	}
	friend class Singleton<SwitcherooCtlClient>;

  public:
	/**
	 * @brief Retrieves a list of available GPUs.
	 *
	 * This function queries the system for all detected GPUs and returns their information
	 * encapsulated in a vector of GPUInfo objects.
	 *
	 * @return A vector containing GPUInfo objects for each detected GPU.
	 */
	const std::vector<GPUInfo> getGpus();
};