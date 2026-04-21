#include <iostream>

#include "clients/unix_socket/rog_perf_tuner_client.hpp"
#include "framework/logger/logger_provider.hpp"

inline void nextProfile() {
	LoggerProvider::initialize();
	std::cout << RogPerfTunerClient::getInstance().nextProfile() << std::endl;
}

inline void nextEffect() {
	LoggerProvider::initialize();
	std::cout << RogPerfTunerClient::getInstance().nextEffect() << std::endl;
}

inline void decreaseBrightness() {
	LoggerProvider::initialize();
	std::cout << RogPerfTunerClient::getInstance().decreaseBrightness() << std::endl;
}

inline void increaseBrightness() {
	LoggerProvider::initialize();
	std::cout << RogPerfTunerClient::getInstance().increaseBrightness() << std::endl;
}