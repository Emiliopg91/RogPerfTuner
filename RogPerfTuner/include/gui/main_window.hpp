#pragma once

#include <qlabel.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>

#include "framework/gui/no_scroll_combo_box.hpp"
#include "framework/translator/translator.hpp"
#include "models/hardware/battery_charge_threshold.hpp"
#include "models/hardware/rgb_brightness.hpp"
#include "models/performance/performance_profile.hpp"
#include "services/application_service.hpp"
#include "services/hardware_service.hpp"
#include "services/open_rgb_service.hpp"
#include "services/performance_service.hpp"
#ifdef STEAM_SUPPORT
#include "services/steam_service.hpp"
#endif
#include "utils/event_bus_wrapper.hpp"

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

#ifdef SCX_SUPPORT
	void setScheduler(std::optional<std::string> sched);
#endif

	void setSsdScheduler(std::string sched);

	void setBatteryChargeLimit(BatteryThreshold value);

	void setAuraBrightness(RgbBrightness brightness);

	void setAuraColor(std::optional<std::string> color);

	void setAuraEffect(std::string effect);

	void onAutostartChanged(bool enabled);

	void onStartMinimizedChanged(bool enabled);

	void onProfileChanged(int index);

#ifdef SCX_SUPPORT
	void onSchedulerChanged(int index);
#endif

	void onSsdSchedulerChanged(int index);

	void onBatteryLimitChanged(int index);

	void onBootSoundChanged(int index);

	void onBootSoundEvent(bool enabled);

	void onEffectChange();

	void onBrightnessChange(int index);

#ifdef STEAM_SUPPORT
	void openGameList();
#endif

	void openFanEditor();

	void showColorPicker();

  private:
	friend class Singleton<MainWindow>;
	int runningGames;
	bool onBattery;

	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
#ifdef STEAM_SUPPORT
	SteamService& steamService = SteamService::getInstance();
#endif
	Translator& translator				   = Translator::getInstance();
	ApplicationService& applicationService = ApplicationService::init(std::nullopt);
	UPowerClient& uPowerClient			   = UPowerClient::getInstance();

	NoScrollComboBox* _profileDropdown;
#ifdef SCX_SUPPORT
	NoScrollComboBox* _schedulerDropdown;
#endif
	NoScrollComboBox* _ssdSchedulerDropdown;
	QPushButton* _gameProfileButton;
	NoScrollComboBox* _effectDropdown;
	NoScrollComboBox* _brightnessDropdown;
	QPushButton* _colorButton;
	NoScrollComboBox* _thresholdDropdown;
	QCheckBox* _autostart;
	QCheckBox* _minimized;
	NoScrollComboBox* _bootSoundDropdown;
	QPushButton* fanEdit;
};
