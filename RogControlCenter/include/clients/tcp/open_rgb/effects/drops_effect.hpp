#pragma once

#include <random>
#include <vector>

#include "../../../../models/others/led_task.hpp"
#include "../../../../models/others/singleton.hpp"
#include "abstract/abstract_effect.hpp"

class DropsEffect : public AbstractEffect, public Singleton<DropsEffect> {
  public:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<DropsEffect>;
	DropsEffect(Client& client);

	void _effect_thread(size_t dev_index, Device& dev);

	LedTask _get_next(size_t dev_index, Device& dev);

	static int random_int(int min, int max);

	static inline std::mt19937 rng{std::random_device{}()};

	std::vector<Color> _available_colors;
	std::vector<std::vector<LedTask>> _buffer;
};
