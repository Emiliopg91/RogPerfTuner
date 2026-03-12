#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"

class IntelRaplUJClient : public AbstractFileClient, public Singleton<IntelRaplUJClient> {
  private:
	IntelRaplUJClient();
	friend class Singleton<IntelRaplUJClient>;

  public:
	void enableRead();
};