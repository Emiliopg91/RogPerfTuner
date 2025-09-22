#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class FanCurveData {
  public:
	std::vector<int> pwm;
	std::vector<int> temp;

	static std::unordered_map<std::string, FanCurveData> parseCurves(std::string);
	std::string toData();
};