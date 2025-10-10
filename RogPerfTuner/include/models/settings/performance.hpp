#pragma once

#include <yaml-cpp/yaml.h>

#include <optional>

#include "../performance/performance_profile.hpp"

struct Performance {
	PerformanceProfile profile			 = PerformanceProfile::Enum::QUIET;
	std::optional<std::string> scheduler = std::nullopt;
};

// YAML-CPP serialization/deserialization
namespace YAML {
template <>
struct convert<Performance> {
	static Node encode(const Performance& perf) {
		Node node;
		node["profile"] = perf.profile.toString();
		if (perf.scheduler.has_value()) {
			node["scheduler"] = perf.scheduler.value();
		}
		return node;
	}

	static bool decode(const Node& node, Performance& perf) {
		if (node["profile"]) {
			perf.profile = PerformanceProfile::fromString(node["profile"].as<std::string>());
		}
		if (node["scheduler"]) {
			perf.scheduler = node["scheduler"].as<std::string>();
		} else {
			perf.scheduler = std::nullopt;
		}
		return true;
	}
};
}  // namespace YAML