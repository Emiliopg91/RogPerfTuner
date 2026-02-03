#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "utils/time_utils.hpp"

struct CPUUsage {
	long long user	  = 0;
	long long nice	  = 0;
	long long system  = 0;
	long long idle	  = 0;
	long long iowait  = 0;
	long long irq	  = 0;
	long long softirq = 0;

	long long total() const {
		return user + nice + system + idle + iowait + irq + softirq;
	}

	long long active() const {
		return total() - idle - iowait;
	}

	static double getUseRate(int interval = 20) {
		CPUUsage cpu1 = CPUUsage::read();
		TimeUtils::sleep(interval);
		CPUUsage cpu2 = CPUUsage::read();

		long long active_diff = cpu2.active() - cpu1.active();
		long long total_diff  = cpu2.total() - cpu1.total();

		return (100.0 * active_diff / total_diff) / 100.0;
	}

	static CPUUsage read() {
		CPUUsage cpu;
		std::ifstream file("/proc/stat");
		std::string line;

		if (file.is_open()) {
			std::getline(file, line);
			std::istringstream ss(line);
			std::string cpu_label;
			ss >> cpu_label >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle >> cpu.iowait >> cpu.irq >> cpu.softirq;
		}

		return cpu;
	}
};