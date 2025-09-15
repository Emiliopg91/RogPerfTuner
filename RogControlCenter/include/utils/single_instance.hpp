#pragma once

#include "../logger/logger.hpp"
#include "../models/others/singleton.hpp"

class SingleInstance : public Singleton<SingleInstance> {
  public:
	void acquire();

  private:
	friend class Singleton<SingleInstance>;
	SingleInstance() {
	}
	Logger logger{"SingleInstance"};
};
