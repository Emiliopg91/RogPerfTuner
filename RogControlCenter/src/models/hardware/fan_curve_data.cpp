#include "../../../include/models/hardware/fan_curve_data.hpp"

#include <regex>
#include <set>
#include <sstream>

#include "../../../include/utils/string_utils.hpp"
#include "spdlog/fmt/bundled/format.h"

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

		data.pwm  = parseNumbers(pwmStr);
		data.temp = parseNumbers(tempStr);

		result[fanName] = data;
	}

	return result;
}

std::string FanCurveData::toData() {
	std::vector<std::string> data;
	std::set<int> addedTemps = {};
	for (size_t i = 0; i < pwm.size(); i++) {
		if (!addedTemps.contains(temp[i])) {
			data.emplace_back(fmt::format("{}c:{}%", temp[i], (100 * pwm[i]) / 255));
			addedTemps.emplace(temp[i]);
		} else {
			auto newTemp = i < pwm.size() - 1 ? (temp[i] + temp[i + 1]) / 2 : temp[i] + 2;
			auto newPwm	 = i < pwm.size() - 1 ? (pwm[i] + pwm[i + 1]) / 2 : pwm[i];

			data.emplace_back(fmt::format("{}c:{}%", newTemp, (100 * newPwm) / 255));
			addedTemps.emplace(newTemp);
		}
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
		int p = std::stoi(entryParts[1].substr(0, entryParts[1].size() - 1)) * 255 / 100;

		tmp.emplace_back(t);
		pwm.emplace_back(p);
	}

	return FanCurveData{pwm, tmp};
}