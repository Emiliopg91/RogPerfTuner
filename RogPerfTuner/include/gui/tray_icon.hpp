#pragma once

#include <qaction.h>

#include <QAction>
#include <QObject>
#include <QSystemTrayIcon>
#include <optional>
#include <string>

#include "framework/translator/translator.hpp"
#include "gui/main_window.hpp"
#include "models/hardware/battery_charge_threshold.hpp"
#include "models/hardware/rgb_brightness.hpp"
#include "models/performance/performance_profile.hpp"
#include "services/application_service.hpp"
#include "services/hardware_service.hpp"
#include "services/open_rgb_service.hpp"
#include "services/performance_service.hpp"
#include "utils/configuration_wrapper.hpp"
#include "utils/event_bus_wrapper.hpp"

class TrayIcon : public QObject, public Singleton<TrayIcon> {
	Q_OBJECT

  public:
	explicit TrayIcon();
	void show();

  public slots:
#ifdef BOOT_SOUND
	void onBootSoundChanged(bool value);
#endif
#ifdef BAT_LIMIT
	void onBatteryLimitChanged(BatteryThreshold value);
#endif
	void onPerformanceProfileChanged(PerformanceProfile value);
	void onEffectChanged(std::string effect);
	void onBrightnessChanged(RgbBrightness brightness);
	void onSchedulerChanged(std::string scheduler);
	void onSsdSchedulerChanged(std::string scheduler);

  private:
	friend class Singleton<TrayIcon>;
	QSystemTrayIcon* tray_icon_;
	QMenu* tray_menu_;

	std::unordered_map<std::string, QAction*> thresholdActions;
	std::unordered_map<std::string, QAction*> brightnessActions;
	std::unordered_map<std::string, QAction*> effectActions;
	std::unordered_map<std::string, QAction*> perfProfileActions;
	std::unordered_map<std::string, QAction*> schedulerActions;
	std::unordered_map<std::string, QAction*> ssdSchedulerActions;
#ifdef BOOT_SOUND
	std::unordered_map<bool, QAction*> bootSoundActions;
#endif
	QMenu* profileMenu;
	QMenu* coolingMenu;
	QMenu* schedulerMenu;
	QMenu* ssdSchedulerMenu;
	QMenu* colorMenu;
	QAction* currentColorAction;
	QAction* pickColorAction;

	void setAuraBrightness(RgbBrightness brightness);
	void setAuraEffect(const std::string& effect);
	void setAuraColor(const std::optional<std::string>& color);
	void setPerformanceProfile(PerformanceProfile profile);
	void setScheduler(std::optional<std::string> scheduler);
	void setSsdScheduler(std::string scheduler);
	void setBatteryThreshold(BatteryThreshold threshold);
#ifdef BOOT_SOUND
	void setBootSound(bool value);
#endif
	void setProfileMenuEnabled();
	void setSchedulerMenuEnabled();
	void setCoolingMenuEnabled(bool enabled);
	void openMainWindow();
#ifdef FAN_CONTROL
	void openFanEditor();
#endif
	void openGameList();

	Shell& shell						   = Shell::getInstance();
	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	ApplicationService& applicationService = ApplicationService::init(std::nullopt);
	Translator& translator				   = Translator::getInstance();
	ConfigurationWrapper& configuration	   = ConfigurationWrapper::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
#ifdef BAT_STATUS
	BatteryStatusClient& batteryStatusClient = BatteryStatusClient::getInstance();
	bool onBattery;
#endif
	MainWindow& mainWindow;

	int runningGames;
};
