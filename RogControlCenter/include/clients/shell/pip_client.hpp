#pragma once

#include "../../models/others/singleton.hpp"
#include "abstract/abstract_cmd_client.hpp"

class PipClient : AbstractCmdClient, public Singleton<PipClient> {
  private:
	PipClient() : AbstractCmdClient("pip", "PipClient") {
	}
	friend class Singleton<PipClient>;

  public:
	void installPackage(std::string package, bool breakSystemPackages = false);
};