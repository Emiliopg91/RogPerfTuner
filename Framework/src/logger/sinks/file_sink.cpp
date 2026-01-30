#include "logger/sink/file_sink.hpp"

#include <stdexcept>

FileSink::FileSink(const std::string& filename) {
	file.open(filename, std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Couldn't open file: " + filename);
	}
}

FileSink::~FileSink() {
	if (file.is_open()) {
		file.close();
	}
}

void FileSink::write(const std::string& message, LoggerLevel _) {
	file.write(message.c_str(), message.length());
	if (!file) {
		throw std::runtime_error("Error writing into file");
	}
	file.flush();
}