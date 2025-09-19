#pragma once

#include "../models/others/loggable.hpp"
#include "../models/others/singleton.hpp"

class SingleInstance : public Singleton<SingleInstance>, Loggable {
  public:
	void acquire();

  private:
	friend class Singleton<SingleInstance>;
	SingleInstance() : Loggable("SingleInstance") {
	}
};
