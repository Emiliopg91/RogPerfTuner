#pragma once

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <map>

#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/performance/performance_profile.hpp"
#include "RccCommons.hpp"

class TrayIcon : public QObject {
	Q_OBJECT

  public:
	explicit TrayIcon(QObject* parent = nullptr);
	inline static TrayIcon* INSTANCE = nullptr;
	void show();

  public slots:
	void setAuraBrightness(RgbBrightness brightness);
	void setAuraEffect(const std::string& effect);
	void setPerformanceProfile(PerformanceProfile profile);
	void setBatteryThreshold(BatteryThreshold threshold);
	void onBatteryLimitChanged(BatteryThreshold value);
	void onPerformanceProfileChanged(PerformanceProfile value);
	void onEffectChanged(std::string effect);
	void onBrightnessChanged(RgbBrightness brightness);

  private:
	QSystemTrayIcon* tray_icon_;
	QMenu* tray_menu_;

	std::map<std::string, QAction*> thresholdActions;
	std::map<std::string, QAction*> brightnessActions;
	std::map<std::string, QAction*> effectActions;
	std::map<std::string, QAction*> perfProfileActions;
};
