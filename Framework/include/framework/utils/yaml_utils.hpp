#pragma once

#include <yaml-cpp/yaml.h>

#include "framework/utils/file_utils.hpp"

class YamlUtils {
  public:
	template <typename T = YAML::Node>
	inline static T parseYaml(std::string content) {
		return YAML::Load(content).as<T>();
	}

	template <typename T = YAML::Node>
	inline static T readYamlFile(std::string path) {
		auto content = FileUtils::readFileContent(path);
		return parseYaml<T>(content);
	}

	template <typename T>
	inline static std::string writeYaml(const T& object) {
		YAML::Emitter out;
		out << YAML::convert<T>::encode(object);
		return out.c_str();
	}

	template <typename T>
	inline static void writeYamlFile(const T& object, std::string path) {
		FileUtils::writeFileContent(path, writeYaml(object));
	}
};