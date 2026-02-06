#pragma once

#include <nlohmann/json.hpp>

#include "framework/utils/file_utils.hpp"

class JsonUtils {
  public:
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