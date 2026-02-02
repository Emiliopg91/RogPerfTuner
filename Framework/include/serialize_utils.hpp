#pragma once

#include <yaml-cpp/yaml.h>

#include <nlohmann/json.hpp>

#include "file_utils.hpp"
#include "yaml-cpp/node/parse.h"

class SerializeUtils {
  public:
	/* ===================== YAML ===================== */

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

	/* ===================== JSON ===================== */

	template <typename T = nlohmann::json>
	inline static T parseJson(const std::string& content) {
		return nlohmann::json::parse(content).get<T>();
	}

	template <typename T = nlohmann::json>
	inline static T readJsonFile(const std::string& path) {
		auto content = FileUtils::readFileContent(path);
		return parseJson<T>(content);
	}

	template <typename T>
	inline static std::string writeJson(const T& object, int indent = 2) {
		nlohmann::json j = object;
		return j.dump(indent);
	}

	template <typename T>
	inline static void writeJsonFile(const T& object, const std::string& path, int indent = 2) {
		FileUtils::writeFileContent(path, writeJson(object, indent));
	}
};