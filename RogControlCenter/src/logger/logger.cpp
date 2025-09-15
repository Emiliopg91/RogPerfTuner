#include "../../include/logger/logger.hpp"

void Logger::setLevel(std::string levelStr) {
	setLevel(spdlog::level::from_str(StringUtils::toLowerCase(levelStr)));
}

/**
 * @brief Set the Level object
 *
 * @param level
 */
void Logger::setLevel(spdlog::level::level_enum level) {
	logger->set_level(level);
	logger->flush_on(level);
}

/**
 * @brief Construct a new Logger object
 *
 * @param name
 */
Logger::Logger(std::string name) {
	logger = LoggerProvider::getLogger(name);
}

void Logger::add_tab() {
	std::lock_guard<std::mutex> lock(mutex);
	tabs += 1;
}

void Logger::rem_tab() {
	std::lock_guard<std::mutex> lock(mutex);
	tabs -= 1;
}