#pragma once

#include "abstracts/singleton.hpp"
#include "clients/file/abstract/abstract_file_client.hpp"

class CPUInfoClient : public AbstractFileClient, public Singleton<CPUInfoClient> {
  private:
	CPUInfoClient();
	friend class Singleton<CPUInfoClient>;
};