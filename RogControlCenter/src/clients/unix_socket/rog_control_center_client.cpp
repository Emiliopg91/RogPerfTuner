#include "../../../include/clients/unix_socket/rog_control_center_client.hpp"

#include <any>

#include "../../../include/utils/constants.hpp"
#include "nlohmann/json_fwd.hpp"

RogControlCenterClient::RogControlCenterClient() : AbstractUnixSocketClient(Constants::SOCKET_FILE, "RogControlCenterClient") {
}

void RogControlCenterClient::nextEffect() {
	invoke(Constants::NEXT_EFF, {});
}

void RogControlCenterClient::increaseBrightness() {
	invoke(Constants::INC_BRIGHT, {});
}

void RogControlCenterClient::decreaseBrightness() {
	invoke(Constants::DEC_BRIGHT, {});
}

SteamGameConfig RogControlCenterClient::getGameConfig(std::string steamId) {
	auto res = invoke(Constants::GAME_CFG, {steamId});

	auto json		 = std::any_cast<std::string>(res[0]);
	nlohmann::json j = nlohmann::json::parse(json);

	SteamGameConfig cfg;
	from_json(j, cfg);

	return cfg;
}