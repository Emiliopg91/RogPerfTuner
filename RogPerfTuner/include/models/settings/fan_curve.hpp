#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct FanCurve {
	std::string current = {};
	std::string factory = {};
};

inline void to_json(nlohmann::json& j, const FanCurve& o) {
	j			 = nlohmann::json{};
	j["current"] = o.current;
	j["factory"] = o.factory;
}

inline void from_json(const nlohmann::json& j, FanCurve& o) {
	if (j.contains("current")) {
		o.current = j.at("current").get<std::string>();
	}
	if (j.contains("factory")) {
		o.factory = j.at("factory").get<std::string>();
	}
}