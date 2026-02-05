#pragma once

#include <random>
#include <vector>

#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"
#include "framework/abstracts/singleton.hpp"

class DanceFloorEffect : public AbstractEffect, public Singleton<DanceFloorEffect> {
  private:
	friend class Singleton<DanceFloorEffect>;
	std::mt19937 _rng;

	Color _get_random_color();

	std::vector<Color> _get_random_colors(size_t length);

  public:
	DanceFloorEffect(Client& client);

  protected:
	void apply_effect(const DeviceList& devices);
};
