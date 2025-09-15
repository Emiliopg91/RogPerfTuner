#include "../../../include/clients/websocket/steam_client.hpp"

#include "../../../include/utils/constants.hpp"

SteamClient::SteamClient() : AbstractWebsocketClient("localhost", Constants::WS_PORT, "SteamClient") {
}

void SteamClient::onGameLaunch(CallbackWithParams&& callback) {
	on_with_params("launch_game", std::move(callback));
}

void SteamClient::onGameStop(CallbackWithParams&& callback) {
	on_with_params("stop_game", std::move(callback));
}

const std::vector<SteamGameDetails> SteamClient::getAppsDetails(const std::vector<unsigned int>& appIds) {
	std::vector<std::any> converted;
	converted.reserve(appIds.size());
	for (auto val : appIds) {
		converted.emplace_back(val);
	}

	std::vector<SteamGameDetails> result;

	auto invResult = invoke("get_apps_details", converted);

	auto map = (std::any_cast<json>(invResult[0])).get<std::unordered_map<std::string, SteamGameDetails>>();
	for (const auto& [key, val] : map) {
		result.emplace_back(val);
	}

	return result;
}

void SteamClient::setLaunchOptions(const int& appid, const std::string& launchOpts) {
	invoke("set_launch_options", {appid, launchOpts});
}

std::string SteamClient::getIcon(const int& appid) {
	return std::any_cast<std::string>(invoke("get_icon", {appid})[0]);
}