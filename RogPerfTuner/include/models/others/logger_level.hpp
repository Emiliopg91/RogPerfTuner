#pragma once

#include <map>
#include <stdexcept>

#include "../../utils/enum_utils.hpp"

enum class LoggerLevel : int { OFF = 0, CRITICAL = 10, ERROR = 20, WARN = 30, INFO = 40, DEBUG = 50 };

namespace LoggerLevelNS {
inline LoggerLevel fromName(const std::string& str) {
	static const std::map<std::string, LoggerLevel> map = {
		{"OFF", LoggerLevel::OFF},	 {"CRITICAL", LoggerLevel::CRITICAL}, {"ERROR", LoggerLevel::ERROR},
		{"WARN", LoggerLevel::WARN}, {"INFO", LoggerLevel::INFO},		  {"DEBUG", LoggerLevel::DEBUG},
	};

	try {
		return map.at(str);
	} catch (const std::out_of_range&) {
		throw std::runtime_error("Unknown log level '" + str + "'");
	}
}

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

constexpr auto toInt(LoggerLevel level) {
	return EnumUtils<LoggerLevel>::toInt(level);
}

constexpr auto toName(LoggerLevel level) {
	return std::string(EnumUtils<LoggerLevel>::toString(level));
}
}  // namespace LoggerLevelNS