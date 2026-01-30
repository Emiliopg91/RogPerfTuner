#pragma once

#include "abstracts/clients/abstract_file_client.hpp"
#include "abstracts/singleton.hpp"

class CPUInfoClient : public AbstractFileClient, public Singleton<CPUInfoClient> {
  private:
	CPUInfoClient();
	friend class Singleton<CPUInfoClient>;
};