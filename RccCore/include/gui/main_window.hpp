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

#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/performance/performance_profile.hpp"
#include "RccCommons.hpp"

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

	void onProfileChanged(int index);

	void onBatteryLimitChanged(int index);

	void onEffectChange();

	void onBrightnessChange(int index);

	void openGameList();

   private:
	Logger* _logger;

	QComboBox* _profileDropdown;
	QPushButton* _gameProfileButton;
	QComboBox* _effectDropdown;
	QComboBox* _brightnessDropdown;
	QComboBox* _thresholdDropdown;
};
