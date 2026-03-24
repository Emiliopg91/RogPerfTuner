#pragma once

#include <iostream>
#include <string>
#include <vector>

struct GPUInfo {
	std::string name;
	bool default_flag;
	std::vector<std::string> environment;
};
inline std::ostream& operator<<(std::ostream& os, const GPUInfo& gpu) {
	os << "Name: " << gpu.name << "\n";
	os << "Default: " << (gpu.default_flag ? "yes" : "no") << "\n";
	os << "Environment: [";
	bool first = true;
	for (const auto& e : gpu.environment) {
		if (!first) {
			os << ", ";
		}
		first = false;
		os << e;
	}
	os << "]\n";
	return os;
}