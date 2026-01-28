#include "clients/shell/asusctl_client.hpp"

#include <unordered_map>

void AsusCtlClient::turnOffAura() {
	run_command("aura static -c 000000", true, false);
}

std::unordered_map<std::string, FanCurveData> AsusCtlClient::getFanCurveData(PlatformProfile profile) {
	auto output = run_command("fan-curve --mod-profile " + formatValue(profile)).stdout_str;

	size_t pos = output.find("[");
	if (pos != output.npos) {
		auto dataStr = output.substr(pos);
		return FanCurveData::parseCurves(dataStr);
	}

	return {};
}

void AsusCtlClient::setCurvesToDefaults(PlatformProfile profile) {
	run_command("fan-curve --mod-profile " + formatValue(profile) + " --default");
}

void AsusCtlClient::setFanCurvesEnabled(PlatformProfile profile, bool enabled) {
	run_command("fan-curve --mod-profile " + formatValue(profile) + " --enable-fan-curves " + (enabled ? "true" : "false"));
}

void AsusCtlClient::setFanCurveData(PlatformProfile profile, std::string fanName, FanCurveData data) {
	setFanCurveStringData(profile, fanName, data.toData());
}

void AsusCtlClient::setFanCurveStringData(PlatformProfile profile, std::string fanName, std::string data) {
	run_command("fan-curve --mod-profile " + formatValue(profile) + " --fan " + fanName + " --data " + data);
}