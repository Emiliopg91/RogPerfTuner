#pragma once

#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"
#include "framework/abstracts/singleton.hpp"

class SpectrumCycleEffect : public AbstractEffect, public Singleton<SpectrumCycleEffect> {
  protected:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<SpectrumCycleEffect>;
	SpectrumCycleEffect(Client& client);
};
