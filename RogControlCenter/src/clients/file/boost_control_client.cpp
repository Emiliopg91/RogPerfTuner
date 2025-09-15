#include "../../../include/clients/file/boost_control_client.hpp"

BoostControlClient::BoostControlClient() {
	std::vector<std::unordered_map<std::string, std::string>> BOOST_CONTROLS = {
		{{"path", "/sys/devices/system/cpu/intel_pstate/no_turbo"}, {"on", "0"}, {"off", "1"}},
		{{"path", "/sys/devices/system/cpu/cpufreq/boost"}, {"on", "1"}, {"off", "0"}}};

	for (auto& file_desc : BOOST_CONTROLS) {
		if (FileUtils::exists(file_desc.at("path"))) {
			client = std::make_unique<BoostControlClientImpl>(file_desc.at("path"), file_desc.at("on"), file_desc.at("off"));
			return;
		}
	}

	client = std::make_unique<BoostControlClientImpl>("", "0", "1");
}

void BoostControlClient::set_boost(bool& enabled) {
	client->set_boost(enabled);
}

bool BoostControlClient::available() {
	return client->available();
}