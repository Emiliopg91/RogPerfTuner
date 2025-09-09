#pragma once

#include "../../models/performance/ssd_scheduler.hpp"
#include "abstract/abstract_file_client.hpp"

class SsdSchedulerClient : public AbstractFileClient {
  public:
	static SsdSchedulerClient& getInstance() {
		static SsdSchedulerClient instance;
		return instance;
	}

	void setScheduler(const SsdScheduler& scheduler) {
		this->write(scheduler.toString());
	}

	const std::vector<SsdScheduler> get_schedulers() {
		std::vector<SsdScheduler> result;
		std::string output = this->read();

		// separar en líneas
		std::istringstream iss(output);
		std::vector<std::string> lines;
		std::string line;
		while (std::getline(iss, line)) {
			lines.push_back(line);
		}

		// iterar sobre cada valor del enum (simulación manual)
		std::vector<SsdScheduler::Enum> all_schedulers = SsdScheduler::getAll();

		for (SsdScheduler sched : all_schedulers) {
			std::string token = sched.toString();
			bool found_in_all = std::all_of(lines.begin(), lines.end(), [&](const std::string& l) { return l.find(token) != std::string::npos; });
			if (found_in_all) {
				result.push_back(sched);
			}
		}

		return result;
	}

  private:
	SsdSchedulerClient() : AbstractFileClient("/sys/block/nvme*/queue/scheduler", "SsdSchedulerClient", true, false) {
	}
};