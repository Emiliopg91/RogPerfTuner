#pragma once

#include "abstract/abstract_effect.hpp"

class BreathingEffect : public AbstractEffect {
  public:
	static BreathingEffect& getInstance(Client& client) {
		static BreathingEffect instance{client};
		return instance;
	}

	void apply_effect(const DeviceList& devices);

  private:
	BreathingEffect(Client& client);

	double _total_time;
	double _pause_time;
	double _frequency;
	int _step_count;
	std::vector<double> _sin_steps;
};