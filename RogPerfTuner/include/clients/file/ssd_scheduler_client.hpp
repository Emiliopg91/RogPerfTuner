#pragma once

#include "../../models/others/singleton.hpp"
#include "../../models/performance/ssd_scheduler.hpp"
#include "abstract/abstract_file_client.hpp"

class SsdSchedulerClient : public AbstractFileClient, public Singleton<SsdSchedulerClient> {
  public:
	/**
	 * @brief Sets the SSD scheduler configuration.
	 *
	 * This method updates the current SSD scheduler with the provided configuration.
	 *
	 * @param scheduler The SsdScheduler object containing the new scheduler settings to apply.
	 */
	void setScheduler(const SsdScheduler& scheduler);

	/**
	 * @brief Retrieves a list of available SSD schedulers.
	 *
	 * This function returns a vector containing all supported SSD scheduler configurations.
	 *
	 * @return A vector of SsdScheduler objects representing the available schedulers.
	 */
	const std::vector<SsdScheduler> get_schedulers();

  private:
	SsdSchedulerClient();
	friend class Singleton<SsdSchedulerClient>;
};