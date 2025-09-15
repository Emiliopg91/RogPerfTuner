/**
 * @file logger_provider.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */

#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "spdlog/async_logger.h"

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
	static std::shared_ptr<spdlog::logger> getLogger(const std::string& name = "Default");

	/**
	 * @brief Set the Config Map object
	 *
	 * @param configMap
	 */
	static void setConfigMap(std::unordered_map<std::string, std::string> configMap);

  private:
	inline static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink{};
	inline static std::optional<std::shared_ptr<spdlog::sinks::basic_file_sink_mt>> file_sink = std::nullopt;
	inline static std::unordered_map<std::string, std::shared_ptr<spdlog::async_logger>> loggers{};

	inline static spdlog::level::level_enum defaultLevel;
	static std::unordered_map<std::string, std::string> configMap;
};