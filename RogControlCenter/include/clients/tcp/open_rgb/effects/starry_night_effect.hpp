#pragma once

#include <random>
#include <thread>
#include <vector>

#include "../../../../models/others/singleton.hpp"
#include "abstract/abstract_effect.hpp"

class StarryNightEffect : public AbstractEffect, public Singleton<StarryNightEffect> {
  private:
	friend class Singleton<StarryNightEffect>;
	int _max_steps = 30;
	std::vector<std::thread> _threads;
	std::mt19937 _rng{std::random_device{}()};

	Color _get_random();

	void _device_effect(Device& device);

	StarryNightEffect(Client& client);

  protected:
	void apply_effect(const DeviceList& devices);
};
