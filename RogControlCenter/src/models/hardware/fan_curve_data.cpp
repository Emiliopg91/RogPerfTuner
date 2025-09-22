#include "../../../include/models/hardware/fan_curve_data.hpp"

#include <regex>
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
	for (size_t i = 0; i < pwm.size(); i++) {
		data.emplace_back(fmt::format("{}c:{}", temp[i], pwm[i]));
	}
	return StringUtils::join(data, ",");
}