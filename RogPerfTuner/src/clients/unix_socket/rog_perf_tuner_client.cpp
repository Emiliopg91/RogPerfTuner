#include "clients/unix_socket/rog_perf_tuner_client.hpp"

#include <any>

#include "framework/utils/yaml_utils.hpp"
#include "models/steam/steam_game_config.hpp"
#include "utils/constants.hpp"

RogPerfTunerClient::RogPerfTunerClient() : AbstractUnixSocketClient(Constants::SOCKET_FILE, "RogPerfTunerClient") {
}

std::string RogPerfTunerClient::nextEffect() {
	return std::any_cast<std::string>(invoke(Constants::NEXT_EFF, {})[0]);
}

std::string RogPerfTunerClient::increaseBrightness() {
	return std::any_cast<std::string>(invoke(Constants::INC_BRIGHT, {})[0]);
}

std::string RogPerfTunerClient::decreaseBrightness() {
	return std::any_cast<std::string>(invoke(Constants::DEC_BRIGHT, {})[0]);
}

std::string RogPerfTunerClient::nextProfile() {
	return std::any_cast<std::string>(invoke(Constants::PERF_PROF, {})[0]);
}

void RogPerfTunerClient::showGui() {
	invoke(Constants::SHOW_GUI, {});
}

SteamGameConfig RogPerfTunerClient::getGameConfig(const std::string& steamId) {
	auto res	  = invoke(Constants::GAME_CFG, {steamId});
	auto yaml_str = std::any_cast<std::string>(res[0]);

	return YamlUtils::parseYaml<SteamGameConfig>(yaml_str);
}