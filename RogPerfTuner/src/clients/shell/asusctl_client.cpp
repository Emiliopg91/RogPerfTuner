#include "clients/shell/asusctl_client.hpp"

#include <regex>
#include <unordered_map>

#include "utils/serialize_utils.hpp"

void AsusCtlClient::turnOffAura() {
	run_command("aura effect static --colour 000000", true, false);
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

std::vector<std::string> AsusCtlClient::getFans(PlatformProfile profile) {
	auto output = run_command("fan-curve --mod-profile " + formatValue(profile)).stdout_str;
	std::vector<std::string> result;

	size_t pos = output.find("[");
	if (pos != output.npos) {
		auto dataStr = output.substr(pos);

		std::regex blockRegex(R"(\(\s*fan:\s*([A-Za-z0-9_]+)\s*,\s*pwm:\s*\(([^)]*)\)\s*,\s*temp:\s*\(([^)]*)\))", std::regex::ECMAScript);

		auto begin = std::sregex_iterator(dataStr.begin(), dataStr.end(), blockRegex);
		auto end   = std::sregex_iterator();

		for (auto it = begin; it != end; ++it) {
			std::smatch match	= *it;
			std::string fanName = match[1].str();
			result.emplace_back(fanName);
		}
	}

	return result;
}