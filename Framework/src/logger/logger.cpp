#include "framework/logger/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>

#include "framework/utils/enum_utils.hpp"

void Logger::setLevel(std::string level) {
	this->setLevel(fromName<LoggerLevel>(level));
}

/**
 * @brief Set the Level object
 *
 * @param level
 */
void Logger::setLevel(LoggerLevel level) {
	this->level = level;
}

/**
 * @brief Construct a new Logger object
 *
 * @param name
 */
Logger::Logger(std::shared_ptr<ConsoleSink> console_sink, std::optional<std::shared_ptr<FileSink>> fileSink, std::string name) {
	std::lock_guard<std::mutex> lock(mutex);
	this->consoleSink = console_sink;
	this->fileSink	  = fileSink;
	this->name		  = StringUtils::rightPad(name, 20).substr(0, 20);
}

/**
 * @brief  Send debug log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::debug(std::string message) {
	log(LoggerLevel::DEBUG, message);
}

/**
 * @brief  Send info log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::info(std::string message) {
	log(LoggerLevel::INFO, message);
}

/**
 * @brief  Send warning log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::warn(std::string message) {
	log(LoggerLevel::WARN, message);
}

/**
 * @brief  Send error log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::error(std::string message) {
	log(LoggerLevel::ERROR, message);
}

/**
 * @brief  Send critical log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::critical(std::string message) {
	log(LoggerLevel::CRITICAL, message);
}

void Logger::add_tab() {
	std::lock_guard<std::mutex> lock(mutex);
	tabs += 1;
}

void Logger::rem_tab() {
	std::lock_guard<std::mutex> lock(mutex);
	tabs = std::max(0, tabs - 1);
}

void Logger::log(LoggerLevel msgLevel, const std::string& format) {
	if (toInt(msgLevel) > toInt(level)) {
		return;
	}

	std::lock_guard<std::mutex> lock(mutex);

	auto out = "[" + now_timestamp() + "][" + StringUtils::rightPad(toName(msgLevel), 7).substr(0, 7) + "][" + name + "] - " +
			   StringUtils::rightPad("", tabs * 2) + format + "\n";
	consoleSink->write(out, msgLevel);
	if (fileSink.has_value()) {
		fileSink.value()->write(out, msgLevel);
	}
}

std::string Logger::now_timestamp() {
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