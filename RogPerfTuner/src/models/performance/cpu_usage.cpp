#include "../../../include/models/performance/cpu_usage.hpp"

long long CPUUsage::total() const {
	return user + nice + system + idle + iowait + irq + softirq;
}

long long CPUUsage::active() const {
	return total() - idle - iowait;
}