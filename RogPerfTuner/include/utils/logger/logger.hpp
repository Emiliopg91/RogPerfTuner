/**
 * @file logger.hpp
 * @author Emiliopg91 (ojosdeserbio@gmail.com)
 * @version 4.0.0
 * @date 2025-09-10
 *
 *
 */
#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <mutex>
#include <optional>

#include "../../models/others/logger_level.hpp"
#include "../string_utils.hpp"
#include "sink/console_sink.hpp"
#include "sink/file_sink.hpp"

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
	Logger(std::shared_ptr<ConsoleSink> consoleSink, std::optional<std::shared_ptr<FileSink>> fileSink, std::string name = "Default");

	/**
	 * @brief  Send debug log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void debug(std::string format) {
		log(LoggerLevel::Enum::DEBUG, format);
	}

	/**
	 * @brief  Send info log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void info(std::string format) {
		log(LoggerLevel::Enum::INFO, format);
	}

	/**
	 * @brief  Send warning log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void warn(std::string format) {
		log(LoggerLevel::Enum::WARN, format);
	}

	/**
	 * @brief  Send error log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void error(std::string format) {
		log(LoggerLevel::Enum::ERROR, format);
	}

	/**
	 * @brief  Send critical log line
	 *
	 * @tparam Args
	 * @param fmt
	 * @param args
	 */
	template <typename... Args>
	void critical(std::string format) {
		log(LoggerLevel::Enum::CRITICAL, format);
	}

	static void add_tab();

	static void rem_tab();

  private:
	inline static int tabs = 0;
	inline static std::mutex mutex;

	inline static std::string now_timestamp() {
		using namespace std::chrono;

		auto now  = system_clock::now();
		auto secs = time_point_cast<seconds>(now);
		auto ms	  = duration_cast<milliseconds>(now - secs).count();

		std::time_t tt = system_clock::to_time_t(secs);
		std::tm tm{};
		localtime_r(&tt, &tm);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms;

		return oss.str();
	}

	std::shared_ptr<ConsoleSink> consoleSink;
	std::optional<std::shared_ptr<FileSink>> fileSink;
	std::string name;
	LoggerLevel level = LoggerLevel::Enum::INFO;

	template <typename... Args>
	void log(LoggerLevel msgLevel, std::string format) {
		std::lock_guard<std::mutex> lock(mutex);
		if (msgLevel.toInt() > this->level.toInt()) {
			return;
		}

		auto out = "[" + now_timestamp() + "][" + StringUtils::rightPad(msgLevel.toName(), 7).substr(0, 7) + "][" + name + "] - " +
				   StringUtils::rightPad("", tabs * 2) + format + "\n";
		consoleSink->write(out);
		if (fileSink.has_value()) {
			fileSink.value()->write(out);
		}
	}
};