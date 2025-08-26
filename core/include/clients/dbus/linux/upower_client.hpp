#pragma once

#include "RccCommons.hpp"

#include "../abstract/abstract_dbus_client.hpp"

class UPowerClient : public AbstractDbusClient
{
public:
    static UPowerClient &getInstance()
    {
        static UPowerClient instance;
        return instance;
    }

    bool isOnBattery()
    {
        return this->getProperty<bool>(QString("OnBattery"));
    }

    template <typename Callback>
    void onBatteryChange(Callback &&callback)
    {
        this->onPropertyChange("OnBattery", callback);
    }

private:
    UPowerClient() : AbstractDbusClient(true,
                                        QString("org.freedesktop.UPower"),
                                        QString("/org/freedesktop/UPower"),
                                        QString("org.freedesktop.UPower"),
                                        true)
    {
    }
};