#pragma once

#include <yaml-cpp/yaml.h>

#include <optional>

#include "framework/utils/enum_utils.hpp"
#include "models/performance/performance_profile.hpp"

struct Performance {
	PerformanceProfile profile			 = PerformanceProfile::SMART;
	std::optional<std::string> scheduler = std::nullopt;
	std::string ssdScheduler			 = "none";
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Performance> {
	static Node encode(const Performance& perf) {
		Node node;
		node["profile"] = toString(perf.profile);
		if (perf.scheduler.has_value()) {
			node["scheduler"] = perf.scheduler.value();
		}
		if (perf.ssdScheduler != "none") {
			node["ssdScheduler"] = perf.ssdScheduler;
		}
		return node;
	}

	static bool decode(const Node& node, Performance& perf) {
		if (node["profile"]) {
			auto profStr = node["profile"].as<std::string>();
			if (profStr == "low_power") {
				profStr = toString(PerformanceProfile::QUIET);
			}
			perf.profile = fromString<PerformanceProfile>(profStr);
		}
		if (node["scheduler"]) {
			perf.scheduler = node["scheduler"].as<std::string>();
		} else {
			perf.scheduler = std::nullopt;
		}
		if (node["ssdScheduler"]) {
			perf.ssdScheduler = node["ssdScheduler"].as<std::string>();
		} else {
			perf.ssdScheduler = "none";
		}
		return true;
	}
};
}  // namespace YAML