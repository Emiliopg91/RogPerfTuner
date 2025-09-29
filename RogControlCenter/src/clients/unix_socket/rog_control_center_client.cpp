#include "../../../include/clients/unix_socket/rog_control_center_client.hpp"

#include <any>

#include "../../../include/utils/constants.hpp"

RogControlCenterClient::RogControlCenterClient() : AbstractUnixSocketClient(Constants::SOCKET_FILE, "RogControlCenterClient") {
}

std::string RogControlCenterClient::nextEffect() {
	auto resp = invoke(Constants::NEXT_EFF, {})[0];
	return std::any_cast<std::string>(resp);
}

RgbBrightness RogControlCenterClient::increaseBrightness() {
	auto resp = invoke(Constants::INC_BRIGHT, {})[0];
	return RgbBrightness::fromInt(std::any_cast<int>(resp));
}

RgbBrightness RogControlCenterClient::decreaseBrightness() {
	auto resp = invoke(Constants::DEC_BRIGHT, {})[0];
	return RgbBrightness::fromInt(std::any_cast<int>(resp));
}

PerformanceProfile RogControlCenterClient::nextProfile() {
	auto resp = invoke(Constants::PERF_PROF, {})[0];
	return PerformanceProfile::fromString(std::any_cast<std::string>(resp));
}

SteamGameConfig RogControlCenterClient::getGameConfiguration(std::string appId) {
	auto arr = invoke(Constants::GAME_CFG, {appId})[0];

	logger.error(std::any_cast<std::string>(arr));
	nlohmann::json j = nlohmann::json::parse(std::any_cast<std::string>(arr));

	SteamGameConfig resp;
	from_json(j, resp);

	return resp;
}