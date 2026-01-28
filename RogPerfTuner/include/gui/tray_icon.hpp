#pragma once

#include <qaction.h>

#include <QAction>
#include <QObject>
#include <QSystemTrayIcon>
#include <optional>
#include <string>

#include "gui/main_window.hpp"
#include "models/hardware/battery_charge_threshold.hpp"
#include "models/hardware/rgb_brightness.hpp"
#include "models/performance/performance_profile.hpp"
#include "services/application_service.hpp"
#include "services/hardware_service.hpp"
#include "services/open_rgb_service.hpp"
#include "services/performance_service.hpp"
#include "utils/configuration/configuration.hpp"
#include "utils/events/event_bus.hpp"
#include "utils/translator/translator.hpp"

class TrayIcon : public QObject, public Singleton<TrayIcon> {
	Q_OBJECT

  public:
	explicit TrayIcon();
	void show();

  public slots:
	void onBootSoundChanged(bool value);
	void onBatteryLimitChanged(BatteryThreshold value);
	void onPerformanceProfileChanged(PerformanceProfile value);
	void onEffectChanged(std::string effect);
	void onBrightnessChanged(RgbBrightness brightness);
	void onSchedulerChanged(std::optional<std::string> scheduler);

  private:
	friend class Singleton<TrayIcon>;
	QSystemTrayIcon* tray_icon_;
	QMenu* tray_menu_;

	std::unordered_map<std::string, QAction*> thresholdActions;
	std::unordered_map<std::string, QAction*> brightnessActions;
	std::unordered_map<std::string, QAction*> effectActions;
	std::unordered_map<std::string, QAction*> perfProfileActions;
	std::unordered_map<std::string, QAction*> schedulerActions;
	std::unordered_map<bool, QAction*> bootSoundActions;
	QMenu* profileMenu;
	QMenu* schedulerMenu;
	QMenu* colorMenu;
	QAction* currentColorAction;
	QAction* pickColorAction;

	void setAuraBrightness(RgbBrightness brightness);
	void setAuraEffect(const std::string& effect);
	void setAuraColor(const std::optional<std::string>& color);
	void setPerformanceProfile(PerformanceProfile profile);
	void setScheduler(std::optional<std::string> scheduler);
	void setBatteryThreshold(BatteryThreshold threshold);
	void setBootSound(bool value);
	void setProfileMenuEnabled();
	void setSchedulerMenuEnabled();
	void openMainWindow();
	void openFanEditor();
	void openSettings();
	void reloadSettings();
	void openLogs();
	void openGameList();

	Shell& shell						   = Shell::getInstance();
	EventBus& eventBus					   = EventBus::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	ApplicationService& applicationService = ApplicationService::init(std::nullopt);
	Translator& translator				   = Translator::getInstance();
	Configuration& configuration		   = Configuration::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
	UPowerClient& uPowerClient			   = UPowerClient::getInstance();
	MainWindow& mainWindow;

	int runningGames;
	bool onBattery;
};
