#pragma once

#include "abstracts/singleton.hpp"
#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"

class BreathingEffect : public AbstractEffect, public Singleton<BreathingEffect> {
  public:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<BreathingEffect>;
	BreathingEffect(Client& client);

	double _total_time;
	double _pause_time;
	double _frequency;
	int _step_count;
	std::vector<double> _sin_steps;
};