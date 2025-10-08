#pragma once

#include "../../../../models/others/singleton.hpp"
#include "abstract/abstract_effect.hpp"

class StaticEffect : public AbstractEffect, public Singleton<StaticEffect> {
  public:
	void apply_effect(const DeviceList& devices);

  private:
	friend class Singleton<StaticEffect>;
	StaticEffect(Client& client);
};