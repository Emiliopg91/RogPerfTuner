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

#include "models/others/logger_level.hpp"
#include "utils/logger/sink/console_sink.hpp"
#include "utils/logger/sink/file_sink.hpp"

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
	void debug(std::string format);

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void debug(format_string_t<Args...> fmt, Args&&... args) {
		debug(std::format(fmt, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void info(std::string format);

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void info(format_string_t<Args...> fmt, Args&&... args) {
		info(std::format(fmt, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send warning log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void warn(std::string format);

	/**
	 * @brief  Send warn log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void warn(format_string_t<Args...> fmt, Args&&... args) {
		warn(std::format(fmt, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void error(std::string format);

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void error(format_string_t<Args...> fmt, Args&&... args) {
		error(std::format(fmt, std::forward<Args>(args)...));
	}

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	void critical(std::string format);

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void critical(format_string_t<Args...> fmt, Args&&... args) {
		critical(std::format(fmt, std::forward<Args>(args)...));
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