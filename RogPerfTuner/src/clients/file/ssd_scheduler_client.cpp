#include "clients/file/ssd_scheduler_client.hpp"

#include <unistd.h>

#include <algorithm>

#include "framework/clients/abstract/abstract_glob_client.hpp"
#include "framework/utils/string_utils.hpp"

SsdSchedulerClient::SsdSchedulerClient() : AbstractGlobClient("/sys/block/nvme*n*/queue/scheduler", "SsdSchedulerClient", true, false) {
	if (available()) {
		logger->info("Initializing SsdSchedulerClient");
		Logger::add_tab();

		auto lines = read();
		for (size_t i = 0; i < lines.size(); i++) {
			auto cleaned = StringUtils::replace(StringUtils::trim(lines[i]), "[", "");
			cleaned		 = StringUtils::replace(cleaned, "]", "");

			auto tmp = StringUtils::split(cleaned, ' ');

			if (i == 0) {
				schedulers = tmp;
				continue;
			}

			for (auto it = schedulers.begin(); it != schedulers.end();) {
				if (std::find(tmp.begin(), tmp.end(), *it) == tmp.end()) {
					it = schedulers.erase(it);
				} else {
					++it;
				}
			}
		}

		logger->info("Available schedulers:");
		Logger::add_tab();
		logger->info(StringUtils::join(schedulers, ", "));
		Logger::rem_tab();

		currentScheduler = "none";

		Logger::rem_tab();
	}
}

std::string SsdSchedulerClient::getCurrentScheduler() {
	return currentScheduler;
}

std::vector<std::string> SsdSchedulerClient::getAvailableSchedulers() {
	return schedulers;
}

void SsdSchedulerClient::setSchedulers(std::string sched) {
	write(sched);
}