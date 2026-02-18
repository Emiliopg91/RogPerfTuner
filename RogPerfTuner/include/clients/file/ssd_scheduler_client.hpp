#pragma once

#include <vector>

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_glob_client.hpp"

class SsdSchedulerClient : public AbstractGlobClient, public Singleton<SsdSchedulerClient> {
  private:
	friend class Singleton<SsdSchedulerClient>;
	SsdSchedulerClient();

	std::vector<std::string> schedulers;
	std::string currentScheduler;

  public:
	std::string getCurrentScheduler();
	std::vector<std::string> getAvailableSchedulers();
	void setSchedulers(std::string);
};