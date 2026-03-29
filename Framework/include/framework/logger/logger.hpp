/**
 * @file logger.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */
#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "framework/logger/sink/console_sink.hpp"
#include "framework/logger/sink/file_sink.hpp"
#include "framework/logger/sink/system_sink.hpp"
#include "framework/models/logger_level.hpp"

template <typename... Args>
using format_string_t = std::format_string<Args...>;

class Logger {
  public:
	/**
	 * @brief Set the Level object
	 *
	 * @param levelStr
	 */
	void setLevel(const std::string& levelStr);

	/**
	 * @brief Set the Level object
	 *
	 * @param level
	 */
	void setLevel(LoggerLevel level);

	/**
	 * @brief Construct a new Logger object
	 *
	 * @param name
	 */
	Logger(std::shared_ptr<ConsoleSink> console_sink, std::optional<std::shared_ptr<FileSink>> fileSink,
		   std::optional<std::shared_ptr<SystemSink>> systemSink, std::string name);

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void debug(const std::string& message);

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void debug(const format_string_t<Args...>& message, Args&&... args) {
		debug(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void info(const std::string& message);

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void info(const format_string_t<Args...>& message, Args&&... args) {
		info(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send warning log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void warn(const std::string& message);

	/**
	 * @brief  Send warn log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void warn(const format_string_t<Args...>& message, Args&&... args) {
		warn(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void error(const std::string& message);

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void error(const format_string_t<Args...>& message, Args&&... args) {
		error(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void critical(const std::string& message);

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void critical(const format_string_t<Args...>& message, Args&&... args) {
		critical(std::format(message, std::forward<Args>(args)...));
	}

	static void add_tab();

	static void rem_tab();

  private:
	inline static int tabs = 0;
	inline static std::mutex mutex;

	std::shared_ptr<ConsoleSink> consoleSink;
	std::optional<std::shared_ptr<FileSink>> fileSink;
	std::optional<std::shared_ptr<SystemSink>> systemSink;
	std::string name;
	LoggerLevel level = LoggerLevel::INFO;

	static std::string now_timestamp();
	void log(LoggerLevel msgLevel, const std::string&);
};