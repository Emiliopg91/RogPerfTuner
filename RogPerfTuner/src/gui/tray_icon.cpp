#include "gui/tray_icon.hpp"

#include <qaction.h>
#include <qcolordialog.h>
#include <qkeysequence.h>
#include <qobject.h>

#include <QActionGroup>
#include <QMenu>
#include <array>
#include <optional>

#include "gui/fan_curve_editor.hpp"
#include "gui/game_list.hpp"
#include "models/performance/performance_profile.hpp"
#include "utils/enum_utils.hpp"
#include "utils/string_utils.hpp"

void TrayIcon::openMainWindow() {
	mainWindow.show();
	mainWindow.raise();
	mainWindow.activateWindow();
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

void TrayIcon::openGameList() {
	if (GameList::INSTANCE == nullptr) {
		GameList::INSTANCE = new GameList(&mainWindow);
	}
	GameList::INSTANCE->show();
}

void TrayIcon::openFanEditor() {
	auto profile		= toString(performanceService.getPerformanceProfile());
	CurveEditor* editor = new CurveEditor(profile, &mainWindow);
	editor->show();
}

void TrayIcon::setAuraBrightness(RgbBrightness brightness) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			brightnessActions[toName(brightness)]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setAuraEffect(const std::string& effect) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			effectActions[effect]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setAuraColor(const std::optional<std::string>& color) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			if (color.has_value()) {
				colorMenu->setEnabled(true);
				currentColorAction->setText(StringUtils::toUpperCase(color.value()).c_str());
			} else {
				colorMenu->setEnabled(false);
			}
		},
		Qt::QueuedConnection);
}

void TrayIcon::setPerformanceProfile(PerformanceProfile profile) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			perfProfileActions[toName(profile)]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setScheduler(std::optional<std::string> scheduler) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			schedulerActions[scheduler.value_or("")]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setBatteryThreshold(BatteryThreshold threshold) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			thresholdActions[toString(threshold)]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setBootSound(bool value) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			bootSoundActions[value]->setChecked(true);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setProfileMenuEnabled() {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			profileMenu->setEnabled(!onBattery && runningGames == 0);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setCoolingMenuEnabled(bool enabled) {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			coolingMenu->setEnabled(enabled);
		},
		Qt::QueuedConnection);
}

void TrayIcon::setSchedulerMenuEnabled() {
	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			schedulerMenu->setEnabled(runningGames == 0);
		},
		Qt::QueuedConnection);
}

// ==============================
// Implementación de slots
// ==============================

void TrayIcon::onBatteryLimitChanged(BatteryThreshold value) {
	hardwareService.setChargeThreshold(value);
}

void TrayIcon::onEffectChanged(std::string effect) {
	openRgbService.setEffect(effect);
}

void TrayIcon::onBrightnessChanged(RgbBrightness brightness) {
	openRgbService.setBrightness(brightness);
}

void TrayIcon::onPerformanceProfileChanged(PerformanceProfile value) {
	if (steamService.getRunningGames().empty()) {
		performanceService.setPerformanceProfile(value);
	}
}

void TrayIcon::onSchedulerChanged(std::optional<std::string> scheduler) {
	if (steamService.getRunningGames().empty()) {
		performanceService.setScheduler(scheduler);
	}
}

// ==============================
// Constructor
// ==============================
TrayIcon::TrayIcon()
	: QObject(&MainWindow::getInstance()),
	  tray_icon_(new QSystemTrayIcon(this)),
	  tray_menu_(new QMenu(&MainWindow::getInstance())),
	  mainWindow(MainWindow::getInstance()) {
	tray_icon_->setIcon(QIcon::fromTheme(Constants::ASSET_ICON_FILE.c_str()));
	tray_icon_->setToolTip(Constants::APP_NAME.c_str());

	QMenu* menu = tray_menu_;

	onBattery	 = uPowerClient.isOnBattery();
	runningGames = steamService.getRunningGames().size();

	// -------------------------
	// Hardware menu
	// -------------------------

	if (hardwareService.getBatteryLimitAvailable() || hardwareService.getBootSoundAvailable()) {
		QAction* hardwareTitle = new QAction(translator.translate("hardware").c_str(), menu);
		hardwareTitle->setEnabled(false);
		menu->addAction(hardwareTitle);
		// -------------------------
		// BatteryThreshold submenu
		// -------------------------
		if (hardwareService.getBatteryLimitAvailable()) {
			QMenu* chargeLimitMenu	  = new QMenu(("    " + translator.translate("charge.threshold")).c_str(), menu);
			QActionGroup* chargeGroup = new QActionGroup(menu);
			for (BatteryThreshold bct : values<BatteryThreshold, true>()) {
				QAction* act = new QAction((std::to_string(toInt(bct)) + "%").c_str(), chargeGroup);
				act->setCheckable(true);
				act->setChecked(bct == hardwareService.getChargeThreshold());
				QObject::connect(act, &QAction::triggered, [this, bct]() {
					onBatteryLimitChanged(bct);
				});
				chargeLimitMenu->addAction(act);
				thresholdActions[toString(bct)] = act;
			}
			menu->insertMenu(nullptr, chargeLimitMenu);

			eventBus.onChargeThreshold([this](BatteryThreshold threshold) {
				setBatteryThreshold(threshold);
			});
		}
		// -------------------------
		// BatteryThreshold submenu
		// -------------------------
		// -------------------------
		// Boot sound submenu
		// -------------------------
		if (hardwareService.getBootSoundAvailable()) {
			QMenu* bootSoundMenu		 = new QMenu(("    " + translator.translate("boot.sound")).c_str(), menu);
			QActionGroup* bootSoundGroup = new QActionGroup(menu);

			const std::array<bool, 2> vals = {true, false};
			for (auto v : vals) {
				QAction* bsAct = new QAction(translator.translate(v ? "enabled" : "disabled").c_str(), bootSoundGroup);
				bsAct->setCheckable(true);
				bsAct->setChecked(v == hardwareService.getBootSound());
				QObject::connect(bsAct, &QAction::triggered, [this, v]() {
					onBootSoundChanged(v);
				});
				bootSoundMenu->addAction(bsAct);
				bootSoundActions[v] = bsAct;
			}

			menu->insertMenu(nullptr, bootSoundMenu);

			eventBus.onBootSound([this](bool value) {
				setBootSound(value);
			});
		}
		// -------------------------
		// Boot sound submenu
		// -------------------------
	}
	// -------------------------
	// Hardware menu
	// -------------------------

	menu->addSeparator();

	// -------------------------
	// Aura menu
	// -------------------------
	QAction* auraTitle = new QAction("Aura", menu);
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

	eventBus.onRgbEffect([this](std::string effect) {
		setAuraEffect(effect);
	});
	// -------------------------
	// Effect submenu
	// -------------------------
	// -------------------------
	// Brightness submenu
	// -------------------------
	QMenu* brightnessMenu		  = new QMenu(("    " + translator.translate("brightness")).c_str(), menu);
	QActionGroup* brightnessGroup = new QActionGroup(menu);
	auto levels					  = values<RgbBrightness, true>();
	for (RgbBrightness item : levels) {
		QAction* act = new QAction(translator.translate("label.brightness." + toName(item)).c_str(), brightnessGroup);
		act->setCheckable(true);
		act->setChecked(item == openRgbService.getCurrentBrightness());
		QObject::connect(act, &QAction::triggered, [this, item]() {
			onBrightnessChanged(item);
		});
		brightnessMenu->addAction(act);
		brightnessActions[toName(item)] = act;
	}
	menu->insertMenu(nullptr, brightnessMenu);

	eventBus.onRgbBrightness([this](RgbBrightness brightness) {
		setAuraBrightness(brightness);
	});
	// -------------------------
	// Brightness submenu
	// -------------------------
	// -------------------------
	// Color submenu
	// -------------------------
	colorMenu = new QMenu(("    " + translator.translate("color")).c_str(), menu);
	colorMenu->setEnabled(openRgbService.getColor().has_value());

	currentColorAction = new QAction(StringUtils::toUpperCase(openRgbService.getColor().value_or("")).c_str(), menu);
	currentColorAction->setEnabled(false);
	colorMenu->addAction(currentColorAction);

	pickColorAction = new QAction((translator.translate("select")).c_str(), menu);
	QObject::connect(pickColorAction, &QAction::triggered, [this]() {
		QColor initial = QColor(openRgbService.getColor().value().c_str());
		QColor chosen  = QColorDialog::getColor(initial, nullptr, translator.translate("pick.color").c_str());

		if (chosen.isValid()) {
			openRgbService.setColor(chosen.name(QColor::HexRgb).toStdString());
		}
	});
	colorMenu->addAction(pickColorAction);

	menu->insertMenu(nullptr, colorMenu);

	eventBus.onRgbColor([this](std::optional<std::string> color) {
		setAuraColor(color);
	});
	// -------------------------
	// Color submenu
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
	auto items				   = values<PerformanceProfile>();
	for (PerformanceProfile item : items) {
		QAction* act = new QAction(translator.translate("label.profile." + toName(item)).c_str(), profileGroup);
		act->setCheckable(true);
		act->setChecked(item == performanceService.getPerformanceProfile());
		QObject::connect(act, &QAction::triggered, [this, item]() {
			onPerformanceProfileChanged(item);
		});
		profileMenu->addAction(act);
		perfProfileActions[toName(item)] = act;
	}

	profileMenu->setEnabled(runningGames == 0 && !onBattery);
	menu->insertMenu(nullptr, profileMenu);
	// -------------------------
	// Profile submenu
	// -------------------------
	// -------------------------
	// Scheduler submenu
	// -------------------------
	if (!performanceService.getAvailableSchedulers().empty()) {
		schedulerMenu			 = new QMenu(("    " + translator.translate("scheduler")).c_str(), menu);
		QActionGroup* schedGroup = new QActionGroup(menu);

		QAction* act = new QAction(performanceService.getDefaultSchedulerName().c_str(), schedGroup);
		act->setCheckable(true);
		act->setChecked(std::nullopt == performanceService.getCurrentScheduler());
		QObject::connect(act, &QAction::triggered, [this]() {
			onSchedulerChanged(std::nullopt);
		});
		schedulerMenu->addAction(act);
		schedulerActions[""] = act;
		schedulerMenu->addSeparator();

		schedulerMenu->setEnabled(runningGames == 0);

		auto items2 = performanceService.getAvailableSchedulers();
		std::reverse(items2.begin(), items2.end());
		for (auto item : items2) {
			act = new QAction(StringUtils::capitalize(item).c_str(), schedGroup);
			act->setCheckable(true);
			act->setChecked(item == performanceService.getCurrentScheduler());
			QObject::connect(act, &QAction::triggered, [this, item]() {
				onSchedulerChanged(item);
			});
			schedulerMenu->addAction(act);
			schedulerActions[item] = act;
		}
		menu->insertMenu(nullptr, schedulerMenu);

		eventBus.onScheduler([this](std::optional<std::string> scheduler) {
			setScheduler(scheduler);
		});
	}
	// -------------------------
	// Scheduler submenu
	// -------------------------
	// -------------------------
	// Fan curves menu
	// -------------------------
	if (!performanceService.getFans().empty()) {
		coolingMenu = new QMenu(("    " + translator.translate("fan.curves")).c_str(), menu);
		menu->insertMenu(nullptr, coolingMenu);

		QAction* act = new QAction((translator.translate("edit.curve") + "...").c_str());
		QObject::connect(act, &QAction::triggered, [this]() {
			openFanEditor();
		});
		coolingMenu->addAction(act);
		coolingMenu->setEnabled(performanceService.getPerformanceProfile() != PerformanceProfile::SMART);
	}
	// -------------------------
	// Fan curves menu
	// -------------------------
	// -------------------------
	// Game submenu
	// -------------------------
	QMenu* gamesMenu = new QMenu(("    " + translator.translate("games")).c_str(), menu);
	menu->insertMenu(nullptr, gamesMenu);

	QAction* act = new QAction((translator.translate("label.game.configure") + "...").c_str());
	QObject::connect(act, &QAction::triggered, [this]() {
		openGameList();
	});
	gamesMenu->addAction(act);

	// -------------------------
	// Game submenu
	// -------------------------
	// -------------------------
	// Performance menu
	// -------------------------

	menu->addSeparator();

	// -------------------------
	// Logs submenu
	// -------------------------
	QMenu* logsMenu = new QMenu(translator.translate("logs").c_str(), menu);
	menu->insertMenu(nullptr, logsMenu);

	QAction* openLogsAct = new QAction(translator.translate("open.log").c_str());
	QObject::connect(openLogsAct, &QAction::triggered, [this]() {
		shell.run_command("xdg-open " + Constants::LOG_DIR + "/" + Constants::LOG_FILE_NAME + ".log", false);
	});
	logsMenu->addAction(openLogsAct);

	QAction* openRunnerLogsAct = new QAction(translator.translate("open.log.runner").c_str());
	QObject::connect(openRunnerLogsAct, &QAction::triggered, [this]() {
		shell.run_command("xdg-open " + Constants::LOG_DIR + "/" + Constants::LOG_RUNNER_FILE_NAME + ".log", false);
	});
	logsMenu->addAction(openRunnerLogsAct);

	QAction* openOpenRGBLogsAct = new QAction(translator.translate("open.log.openrgb").c_str());
	QObject::connect(openOpenRGBLogsAct, &QAction::triggered, [this]() {
		shell.run_command("xdg-open " + Constants::LOG_DIR + "/" + Constants::LOG_ORGB_FILE_NAME + ".log", false);
	});
	logsMenu->addAction(openOpenRGBLogsAct);
	// -------------------------
	// Logs submenu
	// -------------------------

	menu->addSeparator();

	// -------------------------
	// Main window
	// -------------------------

	QAction* openAction = new QAction(translator.translate("open.ui").c_str(), menu);
	QObject::connect(openAction, &QAction::triggered, [this]() {
		openMainWindow();
	});
	menu->addAction(openAction);

	menu->addSeparator();

#ifdef DEV_MODE
	// -------------------------
	// Settings submenu
	// -------------------------
	QMenu* settingsMenu = new QMenu("Settings", menu);
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
#endif

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

	QObject::connect(tray_icon_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			openMainWindow();
		}
	});

	eventBus.onPerformanceProfile([this](PerformanceProfile profile) {
		setPerformanceProfile(profile);
		setCoolingMenuEnabled(profile != PerformanceProfile::SMART);
	});

	eventBus.onGameEvent([this](size_t runningGames) {
		this->runningGames = runningGames;
		setProfileMenuEnabled();
		setSchedulerMenuEnabled();
	});

	eventBus.onBattery([this](bool onBattery) {
		this->onBattery = onBattery;
		setProfileMenuEnabled();
	});
}

void TrayIcon::show() {
	tray_icon_->show();
}

void TrayIcon::onBootSoundChanged(bool enable) {
	hardwareService.setBootSound(enable);
}