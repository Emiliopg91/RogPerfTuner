#pragma once

#include "clients/tcp/open_rgb/effects/abstract/abstract_effect.hpp"
#include "models/others/singleton.hpp"

class StaticEffect : public AbstractEffect, public Singleton<StaticEffect> {
  public:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<StaticEffect>;
	StaticEffect(Client& client);
};