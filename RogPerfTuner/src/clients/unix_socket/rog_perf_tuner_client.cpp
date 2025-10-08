#include "../../../include/clients/unix_socket/rog_perf_tuner_client.hpp"

#include <any>

#include "../../../include/utils/constants.hpp"
#include "nlohmann/json_fwd.hpp"

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

SteamGameConfig RogPerfTunerClient::getGameConfig(std::string steamId) {
	auto res = invoke(Constants::GAME_CFG, {steamId});

	auto json		 = std::any_cast<std::string>(res[0]);
	nlohmann::json j = nlohmann::json::parse(json);

	SteamGameConfig cfg;
	from_json(j, cfg);

	return cfg;
}