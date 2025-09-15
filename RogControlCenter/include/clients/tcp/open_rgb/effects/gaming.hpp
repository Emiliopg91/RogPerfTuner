#pragma once

#include <regex>
#include <vector>

#include "abstract/abstract_effect.hpp"

class GamingEffect : public AbstractEffect {
  public:
	static GamingEffect& getInstance(Client& client) {
		static GamingEffect instance{client};
		return instance;
	}

	static const Color MAIN_COLOR;

	static const std::vector<std::pair<std::regex, Color>> COLOR_MATCHING;

	GamingEffect(Client& client);

	void apply_effect(const DeviceList& devices);
};