#include "../../../include/clients/unix_socket/steam_client.hpp"

#include "../../../include/utils/constants.hpp"

SteamSocketClient::SteamSocketClient() : AbstractUnixSocketClient(Constants::RCDCC_SOCKET_PATH, "SteamClient") {
}

void SteamSocketClient::onGameLaunch(CallbackWithParams&& callback) {
	on_with_params("launch_game", std::move(callback));
}

void SteamSocketClient::onGameStop(CallbackWithParams&& callback) {
	on_with_params("stop_game", std::move(callback));
}

const std::vector<SteamGameDetails> SteamSocketClient::getAppsDetails(const std::vector<unsigned int>& appIds) {
	std::vector<std::any> converted;
	converted.reserve(appIds.size());
	for (auto val : appIds) {
		converted.emplace_back(val);
	}

	std::vector<SteamGameDetails> result;

	auto invResult = invoke("get_apps_details", converted);

	auto yaml_node	= std::any_cast<std::string>(invResult[0]);
	YAML::Node node = YAML::Load(yaml_node);
	auto map		= node.as<std::unordered_map<std::string, SteamGameDetails>>();
	for (const auto& [key, val] : map) {
		result.emplace_back(val);
	}

	return result;
}

void SteamSocketClient::setLaunchOptions(const int& appid, const std::string& launchOpts) {
	invoke("set_launch_options", {appid, launchOpts});
}

std::string SteamSocketClient::getIcon(const int& appid) {
	return std::any_cast<std::string>(invoke("get_icon", {appid})[0]);
}