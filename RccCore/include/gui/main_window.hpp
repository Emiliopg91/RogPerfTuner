#pragma once

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

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

class MainWindow : public QMainWindow {
	Q_OBJECT

  public:
	explicit MainWindow(QWidget* parent = nullptr);
	inline static MainWindow* INSTANCE = nullptr;

  protected:
	void closeEvent(QCloseEvent* event) override;

  private slots:
	void onGameEvent(int runningGames);

	void setPerformanceProfile(PerformanceProfile value);

	void setBatteryChargeLimit(BatteryThreshold value);

	void setAuraBrightness(RgbBrightness brightness);

	void setAuraEffect(std::string effect);

	void onAutostartChanged(bool enabled);

	void onProfileChanged(int index);

	void onBatteryLimitChanged(int index);

	void onEffectChange();

	void onBrightnessChange(int index);

	void openGameList();

  private:
	Logger* _logger;

	EventBus& eventBus					  = EventBus::getInstance();
	ProfileService& profileService		  = ProfileService::getInstance();
	OpenRgbService& openRgbService		  = OpenRgbService::getInstance();
	HardwareService& hardwareService	  = HardwareService::getInstance();
	SteamService& steamService			  = SteamService::getInstance();
	Translator& translator				  = Translator::getInstance();
	ApplicationService applicationService = ApplicationService::getInstance();

	QComboBox* _profileDropdown;
	QPushButton* _gameProfileButton;
	QComboBox* _effectDropdown;
	QComboBox* _brightnessDropdown;
	QComboBox* _thresholdDropdown;
	QCheckBox* _autostart;
};
