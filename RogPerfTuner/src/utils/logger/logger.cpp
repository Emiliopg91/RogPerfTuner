#include "../../../include/utils/logger/logger.hpp"

#include <memory>

void Logger::setLevel(std::string _) {
}

/**
 * @brief Set the Level object
 *
 * @param level
 */
void Logger::setLevel(LoggerLevel _) {
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

void Logger::add_tab() {
	std::lock_guard<std::mutex> lock(mutex);
	tabs += 1;
}

void Logger::rem_tab() {
	std::lock_guard<std::mutex> lock(mutex);
	tabs = std::max(0, tabs - 1);
}