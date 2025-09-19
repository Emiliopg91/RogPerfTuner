#pragma once

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>

#include "../events/event_bus.hpp"
#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/performance/performance_profile.hpp"
#include "../services/application_service.hpp"
#include "../services/hardware_service.hpp"
#include "../services/open_rgb_service.hpp"
#include "../services/profile_service.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"

class MainWindow : public QMainWindow, public Singleton<MainWindow> {
	Q_OBJECT

  public:
	explicit MainWindow(QWidget* parent = nullptr);

  protected:
	void closeEvent(QCloseEvent* event) override;

  private slots:
	void onGameEvent(int runningGames);

	void setPerformanceProfile(PerformanceProfile value);

	void setScheduler(std::optional<std::string> sched);

	void setBatteryChargeLimit(BatteryThreshold value);

	void setAuraBrightness(RgbBrightness brightness);

	void setAuraColor(std::optional<std::string> color);

	void setAuraEffect(std::string effect);

	void onAutostartChanged(bool enabled);

	void onProfileChanged(int index);

	void onSchedulerChanged(int index);

	void onBatteryLimitChanged(int index);

	void onEffectChange();

	void onBrightnessChange(int index);

	void openGameList();

	void showColorPicker();

  private:
	friend class Singleton<MainWindow>;
	Logger* _logger;

	EventBus& eventBus					   = EventBus::getInstance();
	ProfileService& profileService		   = ProfileService::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
	Translator& translator				   = Translator::getInstance();
	ApplicationService& applicationService = ApplicationService::getInstance();

	QComboBox* _profileDropdown;
	QComboBox* _schedulerDropdown;
	QPushButton* _gameProfileButton;
	QComboBox* _effectDropdown;
	QComboBox* _brightnessDropdown;
	QPushButton* _colorButton;
	QComboBox* _thresholdDropdown;
	QCheckBox* _autostart;
};
