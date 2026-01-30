#pragma once

#include "abstracts/singleton.hpp"
#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"

class SpectrumCycleEffect : public AbstractEffect, public Singleton<SpectrumCycleEffect> {
  protected:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<SpectrumCycleEffect>;
	SpectrumCycleEffect(Client& client);
};
