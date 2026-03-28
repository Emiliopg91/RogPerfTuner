#include "framework/logger/sink/system_sink.hpp"

#include "framework/models/logger_level.hpp"

SystemSink::SystemSink(std::string appName) {
	openlog(appName.c_str(), LOG_PID | LOG_CONS, LOG_USER);
}

SystemSink::~SystemSink() {
	closelog();
}
void SystemSink::write(const std::string& message, LoggerLevel level) {
	syslog(toSystemLogLevel(level), "%s", message.c_str());
}