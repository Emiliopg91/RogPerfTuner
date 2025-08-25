#pragma once

#include "RccCommons.hpp"

#include <QSystemTrayIcon>

class TrayIcon
{
public:
    static TrayIcon &getInstance()
    {
        static TrayIcon instance;
        return instance;
    }

private:
    TrayIcon();

    QSystemTrayIcon tray_icon_;
    void onBatteryLimitChanged(BatteryThreshold);
    void onPerformanceProfileChanged(PerformanceProfile);
    void onEffectChanged(std::string);
    void onBrightnessChanged(RgbBrightness);
};