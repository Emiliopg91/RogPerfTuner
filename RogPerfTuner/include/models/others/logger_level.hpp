#pragma once

#include <array>
#include <map>
#include <stdexcept>

#include "../base/int_enum.hpp"
// --------------------
// Meta independiente
// --------------------
struct LoggerLevelMeta {
	enum class Enum : int { OFF = 0, CRITICAL = 10, ERROR = 20, WARN = 30, INFO = 40, DEBUG = 50 } e;
	const char* name;
	int val;
};

class LoggerLevel : public IntEnum<LoggerLevel, LoggerLevelMeta::Enum, LoggerLevelMeta, 6> {
  public:
	using Enum = LoggerLevelMeta::Enum;
	using Base = IntEnum<LoggerLevel, Enum, LoggerLevelMeta, 6>;
	using Base::Base;

	static LoggerLevel from_string(const std::string& str) {
		static const std::map<std::string, LoggerLevel> map = {
			{"OFF", Enum::OFF},	  {"CRITICAL", Enum::CRITICAL}, {"ERROR", Enum::ERROR},
			{"WARN", Enum::WARN}, {"INFO", Enum::INFO},			{"DEBUG", Enum::DEBUG},
		};

		try {
			return map.at(str);
		} catch (const std::out_of_range&) {
			throw std::runtime_error("Unknown log level '" + str + "'");
		}
	}

	const char* colorCode() {
		if (*this == Enum::DEBUG) {
			return "\033[36m";
		}
		if (*this == Enum::INFO) {
			return "\033[32m";
		}
		if (*this == Enum::WARN) {
			return "\033[33m";
		}
		if (*this == Enum::ERROR) {
			return "\033[31m";
		}
		if (*this == Enum::CRITICAL) {
			return "\033[1;31m";
		}
		return "\033[0m";
	}

  private:
	static constexpr std::array<LoggerLevelMeta, 6> table{{{Enum::OFF, "OFF", 0},
														   {Enum::CRITICAL, "CRITICAL", 10},
														   {Enum::ERROR, "ERROR", 20},
														   {Enum::WARN, "WARN", 30},
														   {Enum::INFO, "INFO", 40},
														   {Enum::DEBUG, "DEBUG", 50}}};

	static constexpr const std::array<LoggerLevelMeta, 6>& metaTable() {
		return table;
	}

	friend Base;
};
