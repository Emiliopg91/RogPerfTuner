#include "../../../include/models/others/logger_level.hpp"

const char* colorCode(LoggerLevel level) {
	switch (level) {
		case LoggerLevel::DEBUG:
			return "\033[36m";
		case LoggerLevel::INFO:
			return "\033[32m";
		case LoggerLevel::WARN:
			return "\033[33m";
		case LoggerLevel::ERROR:
			return "\033[31m";
		case LoggerLevel::CRITICAL:
			return "\033[1;31m";
		default:
			return "\033[0m";
	}
}