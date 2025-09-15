#pragma once

#include "../../models/others/singleton.hpp"
#include "abstract/abstract_file_client.hpp"

class CPUInfoClient : public AbstractFileClient, public Singleton<CPUInfoClient> {
  private:
	CPUInfoClient();
	friend class Singleton<CPUInfoClient>;
};