#include "../../include/gui/tray_icon.hpp"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>

#include "../../include/configuration/configuration.hpp"
#include "../../include/events/event_bus.hpp"
#include "../../include/gui/game_list.hpp"
#include "../../include/gui/main_window.hpp"
#include "../../include/models/hardware/battery_charge_threshold.hpp"
#include "../../include/models/hardware/rgb_brightness.hpp"
#include "../../include/models/performance/performance_profile.hpp"
#include "../../include/services/application_service.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "../../include/translator/translator.hpp"

void TrayIcon::openMainWindow() {
	if (!MainWindow::INSTANCE)
		MainWindow::INSTANCE = new MainWindow();

	MainWindow::INSTANCE->show();
}

void TrayIcon::openSettings() {
	shell.run_command("xdg-open " + Constants::CONFIG_FILE);
}

void TrayIcon::reloadSettings() {
	configuration.loadConfig();
}

void TrayIcon::openLogs() {
	shell.run_command("xdg-open " + Constants::LOG_DIR + "/" + Constants::LOG_FILE_NAME + ".log");
}

void openGameList() {
	if (!GameList::INSTANCE) {
		if (MainWindow::INSTANCE)
			GameList::INSTANCE = new GameList(MainWindow::INSTANCE);
		else
			GameList::INSTANCE = new GameList();
	}
	GameList::INSTANCE->show();
}

void TrayIcon::setAuraBrightness(RgbBrightness brightness) {
	QMetaObject::invokeMethod(
		&TrayIcon::getInstance(),
		[=]() {
			brightnessActions[brightness.toName()]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setAuraEffect(const std::string& effect) {
	QMetaObject::invokeMethod(
		&TrayIcon::getInstance(),
		[=]() {
			effectActions[effect]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setPerformanceProfile(PerformanceProfile profile) {
	QMetaObject::invokeMethod(
		&TrayIcon::getInstance(),
		[=]() {
			perfProfileActions[profile.toName()]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setBatteryThreshold(BatteryThreshold threshold) {
	QMetaObject::invokeMethod(
		&TrayIcon::getInstance(),
		[=]() {
			thresholdActions[threshold.toName()]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setProfileMenuEnabled(bool enabled) {
	QMetaObject::invokeMethod(
		&TrayIcon::getInstance(),
		[=]() {
			profileMenu->setEnabled(enabled);
		},
		Qt::QueuedConnection);
}

// ==============================
// Implementación de slots
// ==============================

void TrayIcon::onBatteryLimitChanged(BatteryThreshold value) {
	hardwareService.setChargeThreshold(value);
}

void TrayIcon::onPerformanceProfileChanged(PerformanceProfile value) {
	profileService.setPerformanceProfile(value);
}

void TrayIcon::onEffectChanged(std::string effect) {
	openRgbService.setEffect(effect);
}

void TrayIcon::onBrightnessChanged(RgbBrightness brightness) {
	openRgbService.setBrightness(brightness);
}

// ==============================
// Constructor
// ==============================
TrayIcon::TrayIcon(QObject* parent) : QObject(parent), tray_icon_(new QSystemTrayIcon(this)), tray_menu_(new QMenu()) {
	tray_icon_->setIcon(QIcon::fromTheme(Constants::ASSET_ICON_FILE.c_str()));
	tray_icon_->setToolTip(QString::fromStdString(Constants::APP_NAME + " v" + Constants::APP_VERSION));

	QMenu* menu = tray_menu_;

	// -------------------------
	// Battery menu
	// -------------------------
	QAction* batteryTitle = new QAction(translator.translate("battery").c_str(), menu);
	batteryTitle->setEnabled(false);
	menu->addAction(batteryTitle);
	// -------------------------
	// BatteryThreshold submenu
	// -------------------------
	QMenu* chargeLimitMenu	  = new QMenu(("    " + translator.translate("charge.threshold")).c_str(), menu);
	QActionGroup* chargeGroup = new QActionGroup(menu);
	for (BatteryThreshold bct : BatteryThreshold::getAll()) {
		QAction* act = new QAction((std::to_string(bct.toInt()) + "%").c_str(), chargeGroup);
		act->setCheckable(true);
		act->setChecked(bct == hardwareService.getChargeThreshold());
		QObject::connect(act, &QAction::triggered, [this, bct]() {
			onBatteryLimitChanged(bct);
		});
		chargeLimitMenu->addAction(act);
		thresholdActions[bct.toName()] = act;
	}
	menu->insertMenu(nullptr, chargeLimitMenu);
	// -------------------------
	// BatteryThreshold submenu
	// -------------------------
	// -------------------------
	// Battery menu
	// -------------------------

	menu->addSeparator();

	// -------------------------
	// Aura menu
	// -------------------------
	QAction* auraTitle = new QAction("AuraSync", menu);
	auraTitle->setEnabled(false);
	menu->addAction(auraTitle);
	// -------------------------
	// Effect submenu
	// -------------------------
	QMenu* effectMenu		  = new QMenu(("    " + translator.translate("effect")).c_str(), menu);
	QActionGroup* effectGroup = new QActionGroup(menu);
	auto effects			  = openRgbService.getAvailableEffects();
	for (std::string item : effects) {
		QAction* act = new QAction(item.c_str(), effectGroup);
		act->setCheckable(true);
		act->setChecked(item == openRgbService.getCurrentEffect());
		QObject::connect(act, &QAction::triggered, [this, item]() {
			onEffectChanged(item);
		});
		effectMenu->addAction(act);
		effectActions[item] = act;
	}
	menu->insertMenu(nullptr, effectMenu);
	// -------------------------
	// Effect submenu
	// -------------------------
	// -------------------------
	// Brightness submenu
	// -------------------------
	QMenu* brightnessMenu		  = new QMenu(("    " + translator.translate("brightness")).c_str(), menu);
	QActionGroup* brightnessGroup = new QActionGroup(menu);
	auto levels					  = RgbBrightness::getAll();
	for (RgbBrightness item : levels) {
		QAction* act = new QAction(translator.translate("label.brightness." + item.toName()).c_str(), brightnessGroup);
		act->setCheckable(true);
		act->setChecked(item == openRgbService.getCurrentBrightness());
		QObject::connect(act, &QAction::triggered, [this, item]() {
			onBrightnessChanged(item);
		});
		brightnessMenu->addAction(act);
		brightnessActions[item.toName()] = act;
	}
	menu->insertMenu(nullptr, brightnessMenu);
	// -------------------------
	// Brightness submenu
	// -------------------------
	// -------------------------
	// Aura menu
	// -------------------------

	menu->addSeparator();

	// -------------------------
	// Performance menu
	// -------------------------
	QAction* performanceTitle = new QAction(translator.translate("performance").c_str(), menu);
	performanceTitle->setEnabled(false);
	menu->addAction(performanceTitle);
	// -------------------------
	// Profile submenu
	// -------------------------
	profileMenu				   = new QMenu(("    " + translator.translate("profile")).c_str(), menu);
	QActionGroup* profileGroup = new QActionGroup(menu);
	auto items				   = PerformanceProfile::getAll();
	std::reverse(items.begin(), items.end());
	for (PerformanceProfile item : items) {
		QAction* act = new QAction(translator.translate("label.profile." + item.toName()).c_str(), profileGroup);
		act->setCheckable(true);
		act->setChecked(item == profileService.getPerformanceProfile());
		QObject::connect(act, &QAction::triggered, [this, item]() {
			onPerformanceProfileChanged(item);
		});
		profileMenu->addAction(act);
		perfProfileActions[item.toName()] = act;
	}
	menu->insertMenu(nullptr, profileMenu);
	// -------------------------
	// Profile submenu
	// -------------------------
	// -------------------------
	// Game submenu
	// -------------------------
	QMenu* gamesMenu = new QMenu(("    " + translator.translate("games")).c_str(), menu);
	menu->insertMenu(nullptr, gamesMenu);

	QAction* act = new QAction((translator.translate("label.game.configure") + "...").c_str());
	QObject::connect(act, &QAction::triggered, []() {
		if (!GameList::INSTANCE) {
			if (MainWindow::INSTANCE)
				GameList::INSTANCE = new GameList(MainWindow::INSTANCE);
			else
				GameList::INSTANCE = new GameList();
		}
		GameList::INSTANCE->show();
	});
	gamesMenu->addAction(act);
	// -------------------------
	// Game submenu
	// -------------------------
	// -------------------------
	// Performance menu
	// -------------------------

	menu->addSeparator();

	if (Constants::DEV_MODE) {
		// -------------------------
		// Develop submenu
		// -------------------------
		QAction* developTitle = new QAction("Development", menu);
		developTitle->setEnabled(false);
		menu->addAction(developTitle);
		// -------------------------
		// Settings submenu
		// -------------------------
		QMenu* settingsMenu = new QMenu("    Settings", menu);
		menu->insertMenu(nullptr, settingsMenu);

		QAction* openSettingsAct = new QAction("Open settings");
		QObject::connect(openSettingsAct, &QAction::triggered, [this]() {
			shell.run_command("xdg-open " + Constants::CONFIG_FILE);
		});
		settingsMenu->addAction(openSettingsAct);

		QAction* reloadSettingsAct = new QAction("Reload settings");
		QObject::connect(reloadSettingsAct, &QAction::triggered, [this]() {
			configuration.loadConfig();
		});
		settingsMenu->addAction(reloadSettingsAct);
		// -------------------------
		// Settings submenu
		// -------------------------
		// -------------------------
		// Logs submenu
		// -------------------------
		QMenu* logsMenu = new QMenu("    Logs", menu);
		menu->insertMenu(nullptr, logsMenu);

		QAction* openLogsAct = new QAction("Open logs");
		QObject::connect(openLogsAct, &QAction::triggered, [this]() {
			shell.run_command("xdg-open " + Constants::LOG_DIR + "/" + Constants::LOG_FILE_NAME + ".log");
		});
		logsMenu->addAction(openLogsAct);
		// -------------------------
		// Logs submenu
		// -------------------------
		// -------------------------
		// Develop menu
		// -------------------------
	}

	menu->addSeparator();

	// -------------------------
	// Main window
	// -------------------------

	QAction* openAction = new QAction(translator.translate("open.ui").c_str(), menu);
	QObject::connect(openAction, &QAction::triggered, [this]() {
		openMainWindow();
	});
	menu->addAction(openAction);

	// -------------------------
	// Main window
	// -------------------------

	menu->addSeparator();

	// -------------------------
	// Exit
	// -------------------------
	QAction* quitAction = new QAction(translator.translate("close").c_str(), menu);
	QObject::connect(quitAction, &QAction::triggered, [this]() {
		applicationService.shutdown();
	});
	menu->addAction(quitAction);
	// -------------------------
	// Exit
	// -------------------------

	tray_icon_->setContextMenu(menu);

	eventBus.onRgbBrightness([this](CallbackParam data) {
		setAuraBrightness(std::any_cast<RgbBrightness>(data[0]));
	});

	eventBus.onRgbEffect([this](CallbackParam data) {
		setAuraEffect(std::any_cast<std::string>(data[0]));
	});

	eventBus.onPerformanceProfile([this](CallbackParam data) {
		setPerformanceProfile(std::any_cast<PerformanceProfile>(data[0]));
	});

	eventBus.onChargeThreshold([this](CallbackParam data) {
		setBatteryThreshold(std::any_cast<BatteryThreshold>(data[0]));
	});

	eventBus.onGameEvent([this](CallbackParam data) {
		setProfileMenuEnabled(std::any_cast<size_t>(data[0]) == 0);
	});
}

void TrayIcon::show() {
	tray_icon_->show();
}
