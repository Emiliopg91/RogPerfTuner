#pragma once

#include "clients/file/abstract/abstract_file_client.hpp"
#include "models/others/singleton.hpp"

class CPUInfoClient : public AbstractFileClient, public Singleton<CPUInfoClient> {
  private:
	CPUInfoClient();
	friend class Singleton<CPUInfoClient>;
};