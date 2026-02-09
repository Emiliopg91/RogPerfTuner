#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "framework/utils/time_utils.hpp"

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
		const auto cpu1 = CPUUsage::read();
		TimeUtils::sleep(interval);
		const auto cpu2 = CPUUsage::read();

		const auto active_diff = cpu2.active() - cpu1.active();
		const auto total_diff  = cpu2.total() - cpu1.total();

		if (total_diff <= 0) {
			return 0.0;
		}

		return (static_cast<double>(active_diff) / total_diff) / 100.0;
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