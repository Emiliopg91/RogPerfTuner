#pragma once

#include "abstracts/singleton.hpp"
#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"

class RainbowWave : public AbstractEffect, public Singleton<RainbowWave> {
  public:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<RainbowWave>;
	RainbowWave(Client& client);
};
