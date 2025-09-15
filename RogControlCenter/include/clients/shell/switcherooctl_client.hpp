#pragma once

#include <string>
#include <vector>

#include "../../models/hardware/gpu_info.hpp"
#include "../../models/others/singleton.hpp"
#include "abstract/abstract_cmd_client.hpp"

class SwitcherooCtlClient : AbstractCmdClient, public Singleton<SwitcherooCtlClient> {
  private:
	SwitcherooCtlClient() : AbstractCmdClient("switcherooctl", "SwitcherooCtlClient") {
	}
	friend class Singleton<SwitcherooCtlClient>;

  public:
	const std::vector<GPUInfo> getGpus();
};