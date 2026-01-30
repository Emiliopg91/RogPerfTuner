#include "models/hardware/fan_curve_data.hpp"

#include <regex>
#include <sstream>

#include "string_utils.hpp"

std::unordered_map<std::string, FanCurveData> FanCurveData::parseCurves(std::string input) {
	std::unordered_map<std::string, FanCurveData> result;

	std::regex blockRegex(R"(\(\s*fan:\s*([A-Za-z0-9_]+)\s*,\s*pwm:\s*\(([^)]*)\)\s*,\s*temp:\s*\(([^)]*)\))", std::regex::ECMAScript);

	auto begin = std::sregex_iterator(input.begin(), input.end(), blockRegex);
	auto end   = std::sregex_iterator();

	for (auto it = begin; it != end; ++it) {
		std::smatch match	= *it;
		std::string fanName = match[1].str();
		std::string pwmStr	= match[2].str();
		std::string tempStr = match[3].str();

		FanCurveData data;

		auto parseNumbers = [](const std::string& str) {
			std::vector<int> nums;
			std::stringstream ss(str);
			std::string token;
			while (std::getline(ss, token, ',')) {
				try {
					nums.push_back(std::stoi(token));
				} catch (...) {
				}
			}
			return nums;
		};

		auto pwm  = parseNumbers(pwmStr);
		data.perc = {};
		for (auto p : pwm) {
			data.perc.emplace_back(static_cast<int>((100 * p) / 255));
		}
		data.temp = parseNumbers(tempStr);

		result[fanName] = data;
	}

	return result;
}

std::string FanCurveData::toData() const {
	std::vector<std::string> data;
	for (size_t i = 0; i < perc.size(); i++) {
		data.emplace_back(std::to_string(temp[i]) + "c:" + std::to_string(perc[i]) + "%");
	}
	return StringUtils::join(data, ",");
}

FanCurveData FanCurveData::fromData(std::string data) {
	auto entries = StringUtils::split(data, ',');

	std::vector<int> tmp;
	std::vector<int> pwm;

	for (auto entry : entries) {
		auto entryParts = StringUtils::split(entry, ':');

		int t = std::stoi(entryParts[0].substr(0, entryParts[0].size() - 1));
		int p = std::stoi(entryParts[1].substr(0, entryParts[1].size() - 1));

		tmp.emplace_back(t);
		pwm.emplace_back(p);
	}

	return FanCurveData{pwm, tmp};
}