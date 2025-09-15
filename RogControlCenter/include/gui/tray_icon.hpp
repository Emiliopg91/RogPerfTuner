#pragma once

#include <QAction>
#include <QObject>
#include <QSystemTrayIcon>

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

class TrayIcon : public QObject, public Singleton<TrayIcon> {
	Q_OBJECT

  public:
	explicit TrayIcon(QObject* parent = nullptr);
	void show();

  public slots:
	void onBatteryLimitChanged(BatteryThreshold value);
	void onPerformanceProfileChanged(PerformanceProfile value);
	void onEffectChanged(std::string effect);
	void onBrightnessChanged(RgbBrightness brightness);

  private:
	friend class Singleton<TrayIcon>;
	QSystemTrayIcon* tray_icon_;
	QMenu* tray_menu_;

	std::unordered_map<std::string, QAction*> thresholdActions;
	std::unordered_map<std::string, QAction*> brightnessActions;
	std::unordered_map<std::string, QAction*> effectActions;
	std::unordered_map<std::string, QAction*> perfProfileActions;
	QMenu* profileMenu;

	void setAuraBrightness(RgbBrightness brightness);
	void setAuraEffect(const std::string& effect);
	void setPerformanceProfile(PerformanceProfile profile);
	void setBatteryThreshold(BatteryThreshold threshold);
	void setProfileMenuEnabled(bool enabled);
	void openMainWindow();
	void openSettings();
	void reloadSettings();
	void openLogs();

	Shell& shell						   = Shell::getInstance();
	EventBus& eventBus					   = EventBus::getInstance();
	ProfileService& profileService		   = ProfileService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	ApplicationService& applicationService = ApplicationService::getInstance();
	Translator& translator				   = Translator::getInstance();
	Configuration& configuration		   = Configuration::getInstance();
};
