#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"
class ProcModulesClient : private AbstractFileClient, public Singleton<ProcModulesClient> {
  private:
	ProcModulesClient();
	friend class Singleton<ProcModulesClient>;

  public:
	bool isModuleLoaded(std::string);
};