#pragma once

#include <random>
#include <thread>
#include <vector>

#include "abstract/abstract_effect.hpp"

class StarryNightEffect : public AbstractEffect {
  private:
	int _max_steps = 30;
	std::vector<std::thread> _threads;
	std::mt19937 _rng{std::random_device{}()};

	Color _get_random();

	void _device_effect(Device& device);

	StarryNightEffect(Client& client);

  public:
	static StarryNightEffect& getInstance(Client& client) {
		static StarryNightEffect instance{client};
		return instance;
	}

  protected:
	void apply_effect(const DeviceList& devices);
};
