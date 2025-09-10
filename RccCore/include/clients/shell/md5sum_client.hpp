#pragma once

#include <string>

#include "./abstract/abstract_cmd_client.hpp"

class Md5SumClient : public AbstractCmdClient {
  private:
	Md5SumClient() : AbstractCmdClient("md5sum", "Md5SumClient") {
	}

  public:
	static Md5SumClient& getInstance() {
		static Md5SumClient instance;
		return instance;
	}

	std::string getChecksum(const std::string& file) {
		return StringUtils::split(run_command(file, true).stdout_str, ' ')[0];
	}
};