#pragma once

#include <qlabel.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>

#include "framework/gui/no_scroll_combo_box.hpp"
#include "framework/translator/translator.hpp"
#ifdef BAT_LIMIT
#include "models/hardware/battery_charge_threshold.hpp"
#endif
#include "models/hardware/rgb_brightness.hpp"
#include "models/performance/performance_profile.hpp"
#include "services/application_service.hpp"
#include "services/hardware_service.hpp"
#include "services/open_rgb_service.hpp"
#include "services/performance_service.hpp"
#include "services/steam_service.hpp"
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

	void setScheduler(const std::optional<std::string>& sched);

	void setSsdScheduler(const std::string& sched);

	void setAuraBrightness(RgbBrightness brightness);

	void setAuraColor(std::optional<std::string> color);

	void setAuraEffect(const std::string& effect);

	void onAutostartChanged(bool enabled);

	void onStartMinimizedChanged(bool enabled);

	void onProfileChanged(int index);

	void onSchedulerChanged(int index);

	void onSsdSchedulerChanged(int index);

#ifdef BAT_LIMIT
	void onBatteryLimitChanged(int index);

	void setBatteryChargeLimit(BatteryThreshold value);
#endif

#ifdef BOOT_SOUND
	void onBootSoundChanged(int index);

	void onBootSoundEvent(bool enabled);
#endif

	void onEffectChange();

	void onBrightnessChange(int index);

	void openGameList();

#ifdef FAN_CONTROL
	void openFanEditor();
#endif

	void showColorPicker();

  private:
	friend class Singleton<MainWindow>;
	int runningGames;

	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
	Translator& translator				   = Translator::getInstance();
	ApplicationService& applicationService = ApplicationService::init(std::nullopt);
#ifdef BAT_STATUS
	BatteryStatusClient& batteryStatusClient = BatteryStatusClient::getInstance();
	bool onBattery;
#endif

	NoScrollComboBox* _profileDropdown;
	NoScrollComboBox* _schedulerDropdown;
	NoScrollComboBox* _ssdSchedulerDropdown;
	QPushButton* _gameProfileButton;
	NoScrollComboBox* _effectDropdown;
	NoScrollComboBox* _brightnessDropdown;
	QPushButton* _colorButton;
	QCheckBox* _autostart;
	QCheckBox* _minimized;
#ifdef BOOT_SOUND
	NoScrollComboBox* _bootSoundDropdown;
#endif
#ifdef BAT_LIMIT
	NoScrollComboBox* _thresholdDropdown;
#endif
	QPushButton* fanEdit;
};
