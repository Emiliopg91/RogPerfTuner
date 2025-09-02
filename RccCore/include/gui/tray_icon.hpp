#pragma once

#include <QSystemTrayIcon>
#include <QAction>

#include "RccCommons.hpp"
#include "../models/battery_charge_threshold.hpp"
#include "../models/performance_profile.hpp"
#include "../models/rgb_brightness.hpp"

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

    std::map<std::string, QAction *> thresholdActions;
    std::map<std::string, QAction *> brightnessActions;
    std::map<std::string, QAction *> effectActions;
    std::map<std::string, QAction *> perfProfileActions;

    void setAuraBrightness(RgbBrightness);
    void setAuraEffect(std::string);
    void setPerformanceProfile(PerformanceProfile);
    void setBatteryThreshold(BatteryThreshold threshold);
};