#pragma once

#include "abstract/abstract_effect.hpp"

class RainbowWave : public AbstractEffect {
  public:
	static RainbowWave& getInstance(Client& client) {
		static RainbowWave instance{client};
		return instance;
	}

	void apply_effect(const DeviceList& devices);

  private:
	RainbowWave(Client& client);
};
