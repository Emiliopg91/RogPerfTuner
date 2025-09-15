#pragma once

#include "../logger/logger.hpp"

class SingleInstance {
  public:
	static SingleInstance& getInstance() {
		static SingleInstance instance;
		return instance;
	}

	void acquire();

  private:
	SingleInstance() {
	}
	Logger logger{"SingleInstance"};
};
