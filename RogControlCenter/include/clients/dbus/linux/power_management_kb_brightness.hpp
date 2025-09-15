#pragma once

#include "../../../models/others/singleton.hpp"
#include "../abstract/abstract_dbus_client.hpp"

class PMKeyboardBrightness : public AbstractDbusClient, public Singleton<PMKeyboardBrightness> {
  public:
	int getKeyboardBrightness();

	void setKeyboardBrightnessSilent(const int& brightness);

	void onBrightnessChange(Callback&& callback);

  private:
	friend class Singleton<PMKeyboardBrightness>;
	PMKeyboardBrightness();
};