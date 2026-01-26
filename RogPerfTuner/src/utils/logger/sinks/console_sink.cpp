#include "../../../../include/utils/logger/sink/console_sink.hpp"

#include <iostream>

void ConsoleSink::write(std::string message, LoggerLevel level) {
	std::cout << colorCode(level) << message << "\033[0m";
	std::cout.flush();
}