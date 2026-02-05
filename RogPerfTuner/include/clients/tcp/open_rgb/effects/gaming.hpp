#pragma once

#include <regex>
#include <vector>

#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"
#include "framework/abstracts/singleton.hpp"

class GamingEffect : public AbstractEffect, public Singleton<GamingEffect> {
  private:
	friend class Singleton<GamingEffect>;

  public:
	static const Color MAIN_COLOR;

	static const std::vector<std::pair<std::regex, Color>> COLOR_MATCHING;

	GamingEffect(Client& client);

	void apply_effect(const DeviceList& devices);
};