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

#include "logger/sink/console_sink.hpp"
#include "logger/sink/file_sink.hpp"
#include "models/logger_level.hpp"

template <typename... Args>
using format_string_t = std::format_string<Args...>;

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
	void setLevel(LoggerLevel level);

	/**
	 * @brief Construct a new Logger object
	 *
	 * @param name
	 */
	Logger(std::shared_ptr<ConsoleSink> consoleSink, std::optional<std::shared_ptr<FileSink>> fileSink, std::string name);

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void debug(std::string message);

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void debug(format_string_t<Args...> message, Args&&... args) {
		debug(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void info(std::string message);

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void info(format_string_t<Args...> message, Args&&... args) {
		info(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send warning log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void warn(std::string message);

	/**
	 * @brief  Send warn log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void warn(format_string_t<Args...> message, Args&&... args) {
		warn(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void error(std::string message);

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void error(format_string_t<Args...> message, Args&&... args) {
		error(std::format(message, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void critical(std::string message);

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void critical(format_string_t<Args...> message, Args&&... args) {
		critical(std::format(message, std::forward<Args>(args)...));
	}

	static void add_tab();

	static void rem_tab();

  private:
	inline static int tabs = 0;
	inline static std::mutex mutex;

	std::shared_ptr<ConsoleSink> consoleSink;
	std::optional<std::shared_ptr<FileSink>> fileSink;
	std::string name;
	LoggerLevel level = LoggerLevel::INFO;

	static std::string now_timestamp();
	void log(LoggerLevel msgLevel, const std::string&);
};