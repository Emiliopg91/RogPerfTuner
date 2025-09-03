#pragma once

#include "../asus_base_client.hpp"

class ArmouryBaseClient : public AsusBaseClient {
   public:
	ArmouryBaseClient(std::string object_path_sufix, bool required = false)
		: AsusBaseClient("AsusArmoury", "asus_armoury/" + object_path_sufix, required) {
	}

	int getMinValue() {
		return this->getProperty<int>(QString("MinValue"));
	}

	int getMaxValue() {
		return this->getProperty<int>(QString("MaxValue"));
	}

	int getCurrentValue() {
		return this->getProperty<int>(QString("CurrentValue"));
	}

	void setCurrentValue(int value) {
		this->setProperty<int>(QString("CurrentValue"), value);
	}
};