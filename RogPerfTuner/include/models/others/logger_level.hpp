#pragma once

#include <array>

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
