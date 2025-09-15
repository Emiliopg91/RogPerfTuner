#pragma once

#include "../../models/others/singleton.hpp"
#include "../../models/performance/ssd_scheduler.hpp"
#include "abstract/abstract_file_client.hpp"

class SsdSchedulerClient : public AbstractFileClient, public Singleton<SsdSchedulerClient> {
  public:
	void setScheduler(const SsdScheduler& scheduler);

	const std::vector<SsdScheduler> get_schedulers();

  private:
	SsdSchedulerClient();
	friend class Singleton<SsdSchedulerClient>;
};