#include "logger/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>

#include "enum_utils.hpp"

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
void Logger::debug(std::string format) {
	log(LoggerLevel::DEBUG, format);
}

/**
 * @brief  Send info log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::info(std::string format) {
	log(LoggerLevel::INFO, format);
}

/**
 * @brief  Send warning log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::warn(std::string format) {
	log(LoggerLevel::WARN, format);
}

/**
 * @brief  Send error log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::error(std::string format) {
	log(LoggerLevel::ERROR, format);
}

/**
 * @brief  Send critical log line
 *
 * @tparam Args
 * @param fmt
 * @param args
 */
void Logger::critical(std::string format) {
	log(LoggerLevel::CRITICAL, format);
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
	std::lock_guard<std::mutex> lock(mutex);
	if (toInt(msgLevel) > toInt(level)) {
		return;
	}

	auto out = "[" + now_timestamp() + "][" + StringUtils::rightPad(toName(msgLevel), 7).substr(0, 7) + "][" + name + "] - " +
			   StringUtils::rightPad("", tabs * 2) + format + "\n";
	consoleSink->write(out, level);
	if (fileSink.has_value()) {
		fileSink.value()->write(out, level);
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