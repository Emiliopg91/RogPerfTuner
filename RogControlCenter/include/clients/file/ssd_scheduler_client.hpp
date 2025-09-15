#pragma once

#include "../../models/performance/ssd_scheduler.hpp"
#include "abstract/abstract_file_client.hpp"

class SsdSchedulerClient : public AbstractFileClient {
  public:
	static SsdSchedulerClient& getInstance() {
		static SsdSchedulerClient instance;
		return instance;
	}

	void setScheduler(const SsdScheduler& scheduler);

	const std::vector<SsdScheduler> get_schedulers();

  private:
	SsdSchedulerClient();
};