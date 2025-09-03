#pragma once

#include <iostream>

#include "../utils/string_utils.hpp"
#include "logger_provider.hpp"

template <typename... Args>
using format_string_t = fmt::format_string<Args...>;

class Logger {
  public:
	void setLevel(std::string levelStr) {
		setLevel(spdlog::level::from_str(StringUtils::toLowerCase(levelStr)));
	}

	void setLevel(spdlog::level::level_enum level) {
		logger->set_level(level);
		logger->flush_on(level);
	}

	Logger(std::string name = "Default") {
		logger = LoggerProvider::getLogger(name);
	}

	template <typename... Args>
	void trace(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::trace, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void debug(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void info(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::info, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void warn(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void error(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::err, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void critical(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void log(spdlog::level::level_enum level, format_string_t<Args...> fmt, Args&&... args) {
		std::string padding = "";
		for (int i = 0; i < tabs; i++) {
			padding = "  " + padding;
		}
		std::string formatted = fmt::format(fmt, std::forward<Args>(args)...);
		std::string message	  = fmt::format("{}{}", padding, formatted);
		logger->log(level, message, std::forward<Args>(args)...);
	}

	inline static void add_tab() {
		std::lock_guard<std::mutex> lock(mutex);
		tabs += 1;
	}

	inline static void rem_tab() {
		std::lock_guard<std::mutex> lock(mutex);
		tabs -= 1;
	}

  private:
	inline static int tabs = 0;
	inline static std::mutex mutex;
	std::shared_ptr<spdlog::logger> logger;
};