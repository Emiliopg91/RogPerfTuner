#pragma once

#include "abstract/abstract_effect.hpp"

class StaticEffect : public AbstractEffect {
  public:
	static StaticEffect& getInstance(Client& client) {
		static StaticEffect instance{client};
		return instance;
	}

	void apply_effect(const DeviceList& devices);

  private:
	StaticEffect(Client& client);
};