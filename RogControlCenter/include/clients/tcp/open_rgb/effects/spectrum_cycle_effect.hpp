#pragma once

#include "../../../../models/others/singleton.hpp"
#include "abstract/abstract_effect.hpp"

class SpectrumCycleEffect : public AbstractEffect, public Singleton<SpectrumCycleEffect> {
  protected:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<SpectrumCycleEffect>;
	SpectrumCycleEffect(Client& client);
};
