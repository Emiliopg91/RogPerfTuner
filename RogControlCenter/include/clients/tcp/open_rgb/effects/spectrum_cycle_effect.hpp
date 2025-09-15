#pragma once

#include "abstract/abstract_effect.hpp"

class SpectrumCycleEffect : public AbstractEffect {
  public:
	static SpectrumCycleEffect& getInstance(Client& client) {
		static SpectrumCycleEffect instance{client};
		return instance;
	}

  protected:
	void apply_effect(const DeviceList& devices);

  private:
	SpectrumCycleEffect(Client& client);
};
