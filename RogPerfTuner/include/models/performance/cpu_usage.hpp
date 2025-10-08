#pragma once

struct CPUUsage {
	long long user	  = 0;
	long long nice	  = 0;
	long long system  = 0;
	long long idle	  = 0;
	long long iowait  = 0;
	long long irq	  = 0;
	long long softirq = 0;

	long long total() const;

	long long active() const;
};