/**
 * @file logger_provider.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "framework_constants.hpp"
#include "logger/logger.hpp"
#include "logger/sink/console_sink.hpp"
#include "logger/sink/file_sink.hpp"

class LoggerProvider {
  public:
	/**
	 * @brief Initialize logger provider.
	 *
	 * @param fileName
	 * @param path
	 */
	static void initialize(std::string fileName = "", std::string path = "");

	/**
	 * @brief Get the Logger pointer object
	 *
	 * @param name
	 * @return std::shared_ptr<spdlog::logger>
	 */
	static std::shared_ptr<Logger> getLogger(const std::string& name = FrameworkConstants::DEFAULT_LOGGER_NAME);

	/**
	 * @brief Set the Config Map object
	 *
	 * @param configMap
	 */
	static void setConfigMap(std::map<std::string, LoggerLevel> configMap);

  private:
	inline static std::shared_ptr<ConsoleSink> console_sink{};
	inline static std::optional<std::shared_ptr<FileSink>> file_sink = std::nullopt;
	inline static std::unordered_map<std::string, std::shared_ptr<Logger>> loggers{};

	inline static LoggerLevel defaultLevel = LoggerLevel::INFO;
	static std::map<std::string, LoggerLevel> configMap;
};