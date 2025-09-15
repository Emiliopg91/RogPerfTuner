#pragma once

#include "../abstract/abstract_dbus_client.hpp"

class PMKeyboardBrightness : public AbstractDbusClient {
  public:
	static PMKeyboardBrightness& getInstance() {
		static PMKeyboardBrightness instance;
		return instance;
	}

	int getKeyboardBrightness();

	void setKeyboardBrightnessSilent(const int& brightness);

	void onBrightnessChange(Callback&& callback);

  private:
	PMKeyboardBrightness();
};