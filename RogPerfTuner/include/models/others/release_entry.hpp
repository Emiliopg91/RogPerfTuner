#include <yaml-cpp/yaml.h>

#include <iostream>
#include <string>
#include <vector>

struct ReleaseEntry {
	std::string version;
	std::vector<std::string> features;
	std::vector<std::string> fixes;
	std::vector<std::string> improvements;
};

// Conversión desde YAML → struct
namespace YAML {
template <>
struct convert<ReleaseEntry> {
	static bool decode(const Node& node, ReleaseEntry& entry) {
		if (!node.IsMap()) {
			return false;
		}
		entry.version	   = node["version"].as<std::string>();
		entry.features	   = node["features"] ? node["features"].as<std::vector<std::string>>() : std::vector<std::string>{};
		entry.fixes		   = node["fixes"] ? node["fixes"].as<std::vector<std::string>>() : std::vector<std::string>{};
		entry.improvements = node["improvements"] ? node["improvements"].as<std::vector<std::string>>() : std::vector<std::string>{};
		return true;
	}
};
}  // namespace YAML
