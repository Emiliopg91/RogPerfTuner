/**
 * @file logger.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */
#pragma once

#include <spdlog/spdlog.h>

#include "../utils/string_utils.hpp"
#include "spdlog/common.h"

template <typename... Args>
using format_string_t = fmt::format_string<Args...>;

class Logger {
  public:
	/**
	 * @brief Set the Level object
	 *
	 * @param levelStr
	 */
	void setLevel(std::string levelStr);

	/**
	 * @brief Set the Level object
	 *
	 * @param level
	 */
	void setLevel(spdlog::level::level_enum level);

	/**
	 * @brief Construct a new Logger object
	 *
	 * @param name
	 */
	Logger(std::string name = "Default");

	/**
	 * @brief Send trace log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void trace(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::trace, fmt, std::forward<Args>(args)...);
	}

	/**
	 * @brief Send trace log line
	 *
	 * @param msg
	 * @param args
	 */
	void trace(std::string msg) {
		trace("{}", msg);
	}

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void debug(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
	}

	/**
	 * @brief Send debug log line
	 *
	 * @param msg
	 * @param args
	 */
	void debug(std::string msg) {
		debug("{}", msg);
	}

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void info(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::info, fmt, std::forward<Args>(args)...);
	}

	/**
	 * @brief Send info log line
	 *
	 * @param msg
	 * @param args
	 */
	void info(std::string msg) {
		info("{}", msg);
	}

	/**
	 * @brief  Send warning log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void warn(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
	}

	/**
	 * @brief Send warning log line
	 *
	 * @param msg
	 * @param args
	 */
	void warn(std::string msg) {
		warn("{}", msg);
	}

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void error(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::err, fmt, std::forward<Args>(args)...);
	}

	/**
	 * @brief Send error log line
	 *
	 * @param msg
	 * @param args
	 */
	void error(std::string msg) {
		error("{}", msg);
	}

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void critical(format_string_t<Args...> fmt, Args&&... args) {
		log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
	}

	/**
	 * @brief Send critical log line
	 *
	 * @param msg
	 * @param args
	 */
	void critical(std::string msg) {
		critical("{}", msg);
	}

	static void add_tab();

	static void rem_tab();

  private:
	inline static int tabs = 0;
	inline static std::mutex mutex;
	std::shared_ptr<spdlog::logger> logger;

	template <typename... Args>
	void log(spdlog::level::level_enum level, format_string_t<Args...> fmt, Args&&... args) {
		if (static_cast<int>(logger->level()) > static_cast<int>(level)) {
			return;
		} else {
			logger->log(level, fmt::format("{}{}", StringUtils::leftPad("", tabs * 2), fmt::format(fmt, std::forward<Args>(args)...)));
		}
	}
};