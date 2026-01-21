#include "../../../../include/utils/logger/sink/console_sink.hpp"

#include <iostream>

void ConsoleSink::write(std::string message) {
	std::cout.write(message.c_str(), message.length());
	std::cout.flush();
}