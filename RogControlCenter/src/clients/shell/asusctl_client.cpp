#include "../../../include/clients/shell/asusctl_client.hpp"

#include <unordered_map>

#include "spdlog/fmt/bundled/format.h"

void AsusCtlClient::turnOffAura() {
	run_command("aura static -c 000000", true, false);
}

std::unordered_map<std::string, FanCurveData> AsusCtlClient::getFanCurveData(PlatformProfile profile) {
	auto output = run_command(fmt::format("fan-curve --mod-profile {}", profile.formatValue())).stdout_str;

	size_t pos = output.find("[");
	if (pos != output.npos) {
		auto dataStr = output.substr(pos);
		return FanCurveData::parseCurves(dataStr);
	}

	return {};
}

void AsusCtlClient::setCurvesToDefaults(PlatformProfile profile) {
	run_command(fmt::format("fan-curve --mod-profile {} --default", profile.formatValue()));
}

void AsusCtlClient::setFanCurvesEnabled(PlatformProfile profile, bool enabled) {
	run_command(fmt::format("fan-curve --mod-profile {} --enable-fan-curves {}", profile.formatValue(), enabled));
}

void AsusCtlClient::setFanCurveData(PlatformProfile profile, std::string fanName, FanCurveData data) {
	setFanCurveStringData(profile, fanName, data.toData());
}

void AsusCtlClient::setFanCurveStringData(PlatformProfile profile, std::string fanName, std::string data) {
	run_command(fmt::format("fan-curve --mod-profile {} --fan {} --data {}", profile.formatValue(), fanName, data));
}