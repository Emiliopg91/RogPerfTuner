#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"

class CPUInfoClient : public AbstractFileClient, public Singleton<CPUInfoClient> {
  private:
	CPUInfoClient();
	friend class Singleton<CPUInfoClient>;
};