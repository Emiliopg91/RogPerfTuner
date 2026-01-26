#pragma once

enum class LoggerLevel : int { OFF = 0, CRITICAL = 10, ERROR = 20, WARN = 30, INFO = 40, DEBUG = 50 };

const char* colorCode(LoggerLevel level);