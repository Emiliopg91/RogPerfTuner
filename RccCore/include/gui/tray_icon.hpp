#pragma once

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <map>

#include "../configuration/configuration.hpp"
#include "../events/event_bus.hpp"
#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/performance/performance_profile.hpp"
#include "../services/application_service.hpp"
#include "../services/hardware_service.hpp"
#include "../services/open_rgb_service.hpp"
#include "../services/profile_service.hpp"
#include "../translator/translator.hpp"
#include "RccCommons.hpp"

class TrayIcon : public QObject {
	Q_OBJECT

  public:
	inline static TrayIcon& getInstance() {
		static TrayIcon instance;
		return instance;
	}

	explicit TrayIcon(QObject* parent = nullptr);
	void show();

  public slots:
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

	void setAuraBrightness(RgbBrightness brightness);
	void setAuraEffect(const std::string& effect);
	void setPerformanceProfile(PerformanceProfile profile);
	void setBatteryThreshold(BatteryThreshold threshold);
	void openMainWindow();
	void openSettings();
	void reloadSettings();
	void openLogs();

	Shell& shell						  = Shell::getInstance();
	EventBus& eventBus					  = EventBus::getInstance();
	ProfileService& profileService		  = ProfileService::getInstance();
	OpenRgbService& openRgbService		  = OpenRgbService::getInstance();
	HardwareService& hardwareService	  = HardwareService::getInstance();
	ApplicationService applicationService = ApplicationService::getInstance();
	Translator& translator				  = Translator::getInstance();
	Configuration configuration			  = Configuration::getInstance();
};
