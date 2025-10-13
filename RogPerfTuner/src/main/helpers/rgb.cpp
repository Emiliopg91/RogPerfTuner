#include "../../../include/clients/unix_socket/rog_perf_tuner_client.hpp"
#include "../../../include/logger/logger_provider.hpp"

void nextProfile() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().nextProfile();
}

void nextEffect() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().nextEffect();
}

void decreaseBrightness() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().decreaseBrightness();
}

void increaseBrightness() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().increaseBrightness();
}