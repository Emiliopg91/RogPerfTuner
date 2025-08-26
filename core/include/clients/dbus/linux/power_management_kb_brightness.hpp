#pragma once

#include "../abstract/abstract_dbus_client.hpp"

class PMKeyboardBrightness : public AbstractDbusClient
{
public:
    static PMKeyboardBrightness &getInstance()
    {
        static PMKeyboardBrightness instance;
        return instance;
    }

    int getKeyboardBrightness()
    {
        return call<int>("keyboardBrightness");
    }

    void setKeyboardBrightnessSilent(int brightness)
    {
        call("setKeyboardBrightnessSilent", {brightness});
    }

    template <typename Callback>
    void onBrightnessChange(Callback &&callback)
    {
        this->onSignal("keyboardBrightnessChanged", callback);
    }

private:
    PMKeyboardBrightness()
        : AbstractDbusClient(
              false,
              "org.kde.Solid.PowerManagement",
              "/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl",
              "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl",
              false)
    {
    }
};