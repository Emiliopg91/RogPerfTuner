#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

class LoggerProvider {
   public:
	static void initialize(std::string fileName = "", std::string path = "");
	static std::shared_ptr<spdlog::logger> getLogger(const std::string& name = "Default");
	static void setConfigMap(std::map<std::string, std::string> configMap);

   private:
	inline static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink{};
	inline static std::optional<std::shared_ptr<spdlog::sinks::basic_file_sink_mt>> file_sink = std::nullopt;
	inline static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers{};

	inline static spdlog::level::level_enum defaultLevel;
	static std::map<std::string, std::string> configMap;
};