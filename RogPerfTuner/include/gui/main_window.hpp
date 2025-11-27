#pragma once

#include <qlabel.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>

#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/performance/performance_profile.hpp"
#include "../services/application_service.hpp"
#include "../services/hardware_service.hpp"
#include "../services/open_rgb_service.hpp"
#include "../services/performance_service.hpp"
#include "../services/steam_service.hpp"
#include "../utils/events/event_bus.hpp"
#include "../utils/translator/translator.hpp"

class MainWindow : public QMainWindow, public Singleton<MainWindow> {
	Q_OBJECT

  public:
	explicit MainWindow(QWidget* parent = nullptr);

  protected:
	void closeEvent(QCloseEvent* event) override;

  private slots:
	void onGameEvent();

	void onBatteryEvent();

	void setPerformanceProfile(PerformanceProfile value);

	void setScheduler(std::optional<std::string> sched);

	void setBatteryChargeLimit(BatteryThreshold value);

	void setAuraBrightness(RgbBrightness brightness);

	void setAuraColor(std::optional<std::string> color);

	void setAuraEffect(std::string effect);

	void onAutostartChanged(bool enabled);

	void onStartMinimizedChanged(bool enabled);

	void onProfileChanged(int index);

	void onSchedulerChanged(int index);

	void onBatteryLimitChanged(int index);

	void onBootSoundChanged(int index);

	void onBootSoundEvent(bool enabled);

#ifdef AUR_HELPER
	void onUpdateAvailable(std::string value);

	void onUpdateStart();
#endif

	void onEffectChange();

	void onBrightnessChange(int index);

	void openGameList();

	void openFanEditor();

	void showColorPicker();

  private:
	friend class Singleton<MainWindow>;
	Logger* _logger;
	int runningGames;
	bool onBattery;

	EventBus& eventBus					   = EventBus::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
	Translator& translator				   = Translator::getInstance();
	ApplicationService& applicationService = ApplicationService::init(std::nullopt);
	UPowerClient& uPowerClient			   = UPowerClient::getInstance();

	QComboBox* _profileDropdown;
	QComboBox* _schedulerDropdown;
	QPushButton* _gameProfileButton;
	QComboBox* _effectDropdown;
	QComboBox* _brightnessDropdown;
	QPushButton* _colorButton;
	QComboBox* _thresholdDropdown;
	QCheckBox* _autostart;
	QCheckBox* _minimized;
	QComboBox* _bootSoundDropdown;
	QStatusBar* statusBar;
	QLabel* versionLabel;
	QPushButton* updateButton;
};
