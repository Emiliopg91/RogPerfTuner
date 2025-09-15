#pragma once

#include <string>
#include <vector>

#include "../../models/hardware/gpu_info.hpp"
#include "abstract/abstract_cmd_client.hpp"

class SwitcherooCtlClient : AbstractCmdClient {
  private:
	SwitcherooCtlClient() : AbstractCmdClient("switcherooctl", "SwitcherooCtlClient") {
	}

  public:
	static SwitcherooCtlClient& getInstance() {
		static SwitcherooCtlClient instance;
		return instance;
	}

	const std::vector<GPUInfo> getGpus();
};