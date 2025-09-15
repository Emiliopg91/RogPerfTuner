#pragma once

#include <random>
#include <vector>

#include "abstract/abstract_effect.hpp"

class DanceFloorEffect : public AbstractEffect {
  private:
	std::mt19937 _rng;

	Color _get_random_color();

	std::vector<Color> _get_random_colors(size_t length);

  public:
	static DanceFloorEffect& getInstance(Client& client) {
		static DanceFloorEffect instance{client};
		return instance;
	}
	DanceFloorEffect(Client& client);

  protected:
	void apply_effect(const DeviceList& devices);
};
