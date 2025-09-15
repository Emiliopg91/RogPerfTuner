#pragma once

#include "abstract/abstract_file_client.hpp"

class CPUInfoClient : public AbstractFileClient {
  public:
	static CPUInfoClient& getInstance() {
		static CPUInfoClient instance;
		return instance;
	}

  private:
	CPUInfoClient();
};