#include "../../../include/clients/unix_socket/rog_perf_tuner_client.hpp"

#include <any>

#include "../../../include/utils/constants.hpp"

RogPerfTunerClient::RogPerfTunerClient() : AbstractUnixSocketClient(Constants::SOCKET_FILE, "RogPerfTunerClient") {
}

void RogPerfTunerClient::nextEffect() {
	invoke(Constants::NEXT_EFF, {});
}

void RogPerfTunerClient::increaseBrightness() {
	invoke(Constants::INC_BRIGHT, {});
}

void RogPerfTunerClient::decreaseBrightness() {
	invoke(Constants::DEC_BRIGHT, {});
}

void RogPerfTunerClient::nextProfile() {
	invoke(Constants::PERF_PROF, {});
}

SteamGameConfig RogPerfTunerClient::getGameConfig(std::string steamId) {
	auto res = invoke(Constants::GAME_CFG, {steamId});

	auto yaml_str = std::any_cast<std::string>(res[0]);

	YAML::Node node = YAML::Load(yaml_str);

	SteamGameConfig cfg = node.as<SteamGameConfig>();

	return cfg;
}