#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../models/hardware/gpu_info.hpp"
#include "RccCommons.hpp"
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

	const std::vector<GPUInfo> getGpus() {
		CommandResult result = run_command();
		std::vector<GPUInfo> gpus;
		std::istringstream iss(result.stdout_str);
		std::string line;

		GPUInfo current_gpu;
		bool in_device_block = false;

		while (std::getline(iss, line)) {
			// Elimina espacios al principio y final
			line.erase(0, line.find_first_not_of(" \t"));
			line.erase(line.find_last_not_of(" \t") + 1);

			if (line.empty())
				continue;

			if (line.rfind("Device:", 0) == 0) {
				if (in_device_block) {
					gpus.push_back(current_gpu);  // guarda GPU anterior
				}
				current_gpu		= GPUInfo{};  // resetea
				in_device_block = true;
			} else if (line.rfind("Name:", 0) == 0) {
				current_gpu.name = line.substr(5);
				current_gpu.name.erase(0, current_gpu.name.find_first_not_of(" \t"));
			} else if (line.rfind("Default:", 0) == 0) {
				std::string val = line.substr(8);
				val.erase(0, val.find_first_not_of(" \t"));
				current_gpu.default_flag = (val == "yes");
			} else if (line.rfind("Environment:", 0) == 0) {
				std::string env = line.substr(12);
				env.erase(0, env.find_first_not_of(" \t"));

				std::istringstream env_iss(env);
				std::string token;
				while (env_iss >> token) {
					current_gpu.environment.push_back(token);
				}
			}
		}

		if (in_device_block) {
			gpus.push_back(current_gpu);
		}

		return gpus;
	}
};