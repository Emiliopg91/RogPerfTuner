#pragma once

#include <string>

#include "../../models/others/singleton.hpp"
#include "./abstract/abstract_cmd_client.hpp"

class Md5SumClient : public AbstractCmdClient, public Singleton<Md5SumClient> {
  private:
	Md5SumClient() : AbstractCmdClient("md5sum", "Md5SumClient") {
	}
	friend class Singleton<Md5SumClient>;

  public:
	std::string getChecksum(const std::string& file);
};