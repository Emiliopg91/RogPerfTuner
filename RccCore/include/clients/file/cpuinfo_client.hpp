#pragma once
#include "RccCommons.hpp"
#include "abstract/abstract_file_client.hpp"

class CPUInfoClient : public AbstractFileClient {
  public:
	static CPUInfoClient& getInstance() {
		static CPUInfoClient instance;
		return instance;
	}

  private:
	CPUInfoClient() : AbstractFileClient("/proc/cpuinfo", "CPUInfoClient") {
	}
};