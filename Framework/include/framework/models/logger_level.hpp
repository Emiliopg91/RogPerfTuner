#pragma once

#include <sys/syslog.h>
enum class LoggerLevel { OFF = 0, CRITICAL = 1, ERROR = 2, WARN = 3, INFO = 4, DEBUG = 5 };

inline const char* colorCode(LoggerLevel level) {
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

inline int toSystemLogLevel(LoggerLevel level) {
	switch (level) {
		case LoggerLevel::DEBUG:
			return LOG_DEBUG;
		case LoggerLevel::INFO:
			return LOG_INFO;
		case LoggerLevel::WARN:
			return LOG_WARNING;
		case LoggerLevel::ERROR:
			return LOG_ERR;
		case LoggerLevel::CRITICAL:
			return LOG_CRIT;
		default:
			return LOG_DEBUG;
	}
}