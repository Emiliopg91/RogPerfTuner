#include "../clients/unix_socket/rog_perf_tuner_client.hpp"
#include "../utils/logger/logger_provider.hpp"

inline void nextProfile() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().nextProfile();
}

inline void nextEffect() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().nextEffect();
}

inline void decreaseBrightness() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().decreaseBrightness();
}

inline void increaseBrightness() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().increaseBrightness();
}