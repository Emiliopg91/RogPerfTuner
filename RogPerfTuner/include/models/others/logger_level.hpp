#pragma once

enum class LoggerLevel : int { OFF = 0, CRITICAL = 1, ERROR = 2, WARN = 3, INFO = 4, DEBUG = 5 };

const char* colorCode(LoggerLevel level);