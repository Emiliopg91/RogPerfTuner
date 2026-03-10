#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"

class SchedBoreClient : public AbstractFileClient, public Singleton<SchedBoreClient> {
  private:
	SchedBoreClient();
	friend class Singleton<SchedBoreClient>;
};