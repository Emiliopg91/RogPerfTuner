#pragma once

#include "abstracts/singleton.hpp"
#include "clients/dbus/abstract/abstract_dbus_client.hpp"

class PMKeyboardBrightness : public AbstractDbusClient, public Singleton<PMKeyboardBrightness> {
  public:
	int getKeyboardBrightness();

	void setKeyboardBrightnessSilent(const int& brightness);

	void onBrightnessChange(Callback&& callback);

  private:
	friend class Singleton<PMKeyboardBrightness>;
	PMKeyboardBrightness();
};