#pragma once

#include "../asus_base_client.hpp"

class ArmouryBaseClient : public AsusBaseClient {
  public:
	ArmouryBaseClient(std::string object_path_sufix, bool required = false);

	int getMinValue();

	int getMaxValue();

	int getCurrentValue();

	void setCurrentValue(int value);
};