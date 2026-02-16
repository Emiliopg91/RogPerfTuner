#include "gui/main_window.hpp"

#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <QApplication>
#include <QColorDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>
#include <algorithm>
#include <optional>

#include "OpenRGB/Color.hpp"
#include "framework/utils/string_utils.hpp"
#include "gui/fan_curve_editor.hpp"
#include "gui/game_list.hpp"
#include "models/performance/performance_profile.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	onBattery	 = uPowerClient.isOnBattery();
	runningGames = steamService.getRunningGames().size();

	setWindowTitle((Constants::APP_NAME + " | " + Constants::APP_VERSION).c_str());
	setWindowIcon(QIcon(Constants::ASSET_ICON_45_FILE.c_str()));

	QWidget* centralWidget	= new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
	mainLayout->setContentsMargins(20, 10, 20, 10);
	mainLayout->setAlignment(Qt::AlignTop);

	// -------------------------
	// Logo
	// -------------------------
	QLabel* imageLabel = new QLabel();
	QPixmap pixmap(Constants::ASSET_ICON_FILE.c_str());
	QPixmap scaledPixmap = pixmap.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	imageLabel->setPixmap(scaledPixmap);
	imageLabel->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
	// -------------------------
	// Logo
	// -------------------------

	// -------------------------
	// Performance group
	// -------------------------
	QGroupBox* performanceGroup	   = new QGroupBox(translator.translate("performance").c_str());
	QFormLayout* performanceLayout = new QFormLayout();
	performanceLayout->setContentsMargins(20, 10, 20, 10);
	// -------------------------
	// Profile menu
	// -------------------------
	_profileDropdown = new NoScrollComboBox();
	_profileDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items = values<PerformanceProfile>();
	for (PerformanceProfile item : items) {
		_profileDropdown->addItem(("  " + translator.translate("label.profile." + toName(item))).c_str(), toString(item).c_str());
	}
	connect(_profileDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);
	setPerformanceProfile(performanceService.getPerformanceProfile());
	_profileDropdown->setEnabled(runningGames == 0 && !onBattery);
	performanceLayout->addRow(new QLabel((translator.translate("profile") + ":").c_str()), _profileDropdown);
	// -------------------------
	// Profile menu
	// -------------------------
	// -------------------------
	// Scheduler menu
	// -------------------------
	if (!performanceService.getAvailableSchedulers().empty()) {
		_schedulerDropdown = new NoScrollComboBox();
		_schedulerDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		_schedulerDropdown->addItem(("  " + performanceService.getDefaultSchedulerName()).c_str(), "");

		auto items2 = performanceService.getAvailableSchedulers();
		std::reverse(items2.begin(), items2.end());
		for (auto item : items2) {
			_schedulerDropdown->addItem(("  " + StringUtils::capitalize(item)).c_str(), item.c_str());
		}
		connect(_schedulerDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSchedulerChanged);
		setScheduler(performanceService.getCurrentScheduler());
		_schedulerDropdown->setEnabled(runningGames == 0);
		performanceLayout->addRow(new QLabel((translator.translate("scheduler") + ":").c_str()), _schedulerDropdown);

		eventBus.onScheduler([this](std::optional<std::string> sched) {
			setScheduler(sched);
		});
	}
	// -------------------------
	// Scheduler menu
	// -------------------------
	// -------------------------
	// Fan curves menu
	// -------------------------
	if (!performanceService.getFans().empty()) {
		fanEdit = new QPushButton();
		fanEdit->setText(translator.translate("edit.curve").c_str());
		connect(fanEdit, &QPushButton::clicked, this, &MainWindow::openFanEditor);
		fanEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		fanEdit->setEnabled(performanceService.getPerformanceProfile() != PerformanceProfile::SMART);

		performanceLayout->addRow(new QLabel((translator.translate("fan.curves") + ":").c_str()), fanEdit);
	}
	// -------------------------
	// Fan curves menu
	// -------------------------
	// -------------------------
	// Games menu
	// -------------------------
	_gameProfileButton = new QPushButton(translator.translate("label.game.configure").c_str());
	_gameProfileButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(_gameProfileButton, &QPushButton::clicked, this, &MainWindow::openGameList);
	performanceLayout->addRow(new QLabel((translator.translate("games") + ":").c_str()), _gameProfileButton);
	// -------------------------
	// Games menu
	// -------------------------
	performanceGroup->setLayout(performanceLayout);
	mainLayout->addWidget(performanceGroup);

	eventBus.onPerformanceProfile([this](PerformanceProfile profile) {
		setPerformanceProfile(profile);
		fanEdit->setEnabled(profile != PerformanceProfile::SMART);
	});
	// -------------------------
	// Performance group
	// -------------------------

	// -------------------------
	// Aura group
	// -------------------------
	QGroupBox* auraGroup	= new QGroupBox("Aura");
	QFormLayout* auraLayout = new QFormLayout();
	auraLayout->setContentsMargins(20, 10, 20, 10);
	// -------------------------
	// Effect menu
	// -------------------------
	_effectDropdown = new NoScrollComboBox();
	_effectDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto effects = openRgbService.getAvailableEffects();
	for (const auto effect : effects) {
		_effectDropdown->addItem(("  " + effect).c_str(), effect.c_str());
	}

	auto it = std::find(effects.begin(), effects.end(), openRgbService.getCurrentEffect());
	_effectDropdown->setCurrentIndex(std::distance(effects.begin(), it));
	connect(_effectDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEffectChange);
	auraLayout->addRow(new QLabel((translator.translate("effect") + ":").c_str()), _effectDropdown);

	eventBus.onRgbEffect([this](std::string effect) {
		setAuraEffect(effect);
	});
	// -------------------------
	// Effect menu
	// -------------------------
	// -------------------------
	// Brightness + Color menu
	// -------------------------
	_brightnessDropdown = new NoScrollComboBox();
	_brightnessDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	for (RgbBrightness b : values<RgbBrightness, true>()) {
		_brightnessDropdown->addItem(("  " + translator.translate("label.brightness." + toName(b))).c_str(), toInt(b));
	}
	_brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(toInt(openRgbService.getCurrentBrightness())));
	connect(_brightnessDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBrightnessChange);

	_colorButton = new QPushButton();
	_colorButton->setFixedSize(25, 25);
	_colorButton->setStyleSheet(("background-color: " + (openRgbService.getColor().value_or(Color::Black.toHex()))).c_str());
	_colorButton->setCursor(Qt::PointingHandCursor);
	_colorButton->setEnabled(openRgbService.getColor().has_value());
	connect(_colorButton, &QPushButton::clicked, this, &MainWindow::showColorPicker);

	auto brightnessColorLayout = new QHBoxLayout();
	brightnessColorLayout->addWidget(_brightnessDropdown);
	brightnessColorLayout->addWidget(_colorButton);
	brightnessColorLayout->setSpacing(10);

	auraLayout->addRow(new QLabel((translator.translate("brightness") + ":").c_str()), brightnessColorLayout);

	auraGroup->setLayout(auraLayout);
	mainLayout->addWidget(auraGroup);

	eventBus.onRgbBrightness([this](RgbBrightness brightness) {
		setAuraBrightness(brightness);
	});

	eventBus.onRgbColor([this](std::optional<std::string> color) {
		setAuraColor(color);
	});
	// -------------------------
	// Brightness + Color menu
	// -------------------------
	// -------------------------
	// Aura group
	// -------------------------

	// -------------------------
	// Hardware group
	// -------------------------
	if (hardwareService.getBatteryLimitAvailable() || hardwareService.getBootSoundAvailable()) {
		QGroupBox* hardwareGroup	= new QGroupBox(translator.translate("hardware").c_str());
		QFormLayout* hardwareLayout = new QFormLayout();
		hardwareLayout->setContentsMargins(20, 10, 20, 10);
		// -------------------------
		// Battery menu
		// -------------------------
		if (hardwareService.getBatteryLimitAvailable()) {
			_thresholdDropdown = new NoScrollComboBox();
			_thresholdDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			for (BatteryThreshold t : values<BatteryThreshold, true>()) {
				const auto intVal = toInt(t);
				_thresholdDropdown->addItem("  " + QString::number(intVal) + "%", intVal);
			}
			connect(_thresholdDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBatteryLimitChanged);
			setBatteryChargeLimit(hardwareService.getChargeThreshold());
			hardwareLayout->addRow(new QLabel((translator.translate("charge.threshold") + ":").c_str()), _thresholdDropdown);

			eventBus.onChargeThreshold([this](BatteryThreshold threshold) {
				setBatteryChargeLimit(threshold);
			});
		}
		// -------------------------
		// Battery menu
		// -------------------------
		// -------------------------
		// Boot sound menu
		// -------------------------
		if (hardwareService.getBootSoundAvailable()) {
			_bootSoundDropdown = new NoScrollComboBox();
			_bootSoundDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

			auto vals = std::array<bool, 2>{true, false};
			for (size_t i = 0; i < vals.size(); i++) {
				_bootSoundDropdown->addItem(("  " + translator.translate(vals[i] ? "enabled" : "disabled")).c_str(), vals[i]);
			}
			onBootSoundEvent(hardwareService.getBootSound());

			connect(_bootSoundDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBootSoundChanged);
			hardwareLayout->addRow(new QLabel(translator.translate("boot.sound").c_str()), _bootSoundDropdown);

			eventBus.onBootSound([this](bool value) {
				onBootSoundEvent(value);
			});
		}
		// -------------------------
		// Boot sound menu
		// -------------------------
		hardwareGroup->setLayout(hardwareLayout);
		mainLayout->addWidget(hardwareGroup);
	}
	// -------------------------
	// Hardware group
	// -------------------------

	// -------------------------
	// Settings group
	// -------------------------
	QGroupBox* settingsGroup	= new QGroupBox(translator.translate("settings").c_str());
	QFormLayout* settingsLayout = new QFormLayout();
	settingsLayout->setContentsMargins(20, 10, 20, 10);
	// -------------------------
	// Start minimized
	// -------------------------
	_minimized = new QCheckBox();
	_minimized->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_minimized->setEnabled(true);
	_minimized->setChecked(applicationService.isStartMinimized());
	connect(_minimized, &QCheckBox::toggled, this, &MainWindow::onStartMinimizedChanged);
	settingsLayout->addRow(_minimized, new QLabel(translator.translate("start.minimized").c_str()));
	// -------------------------
	// Start minimized
	// -------------------------
	// -------------------------
	// Autostart
	// -------------------------
	_autostart = new QCheckBox();
	_autostart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
#ifdef DEV_MODE
	_autostart->setEnabled(false);
#endif
	_autostart->setChecked(applicationService.isAutostartEnabled());
	connect(_autostart, &QCheckBox::toggled, this, &MainWindow::onAutostartChanged);
	settingsLayout->addRow(_autostart, new QLabel(translator.translate("autostart").c_str()));
	// -------------------------
	// Autostart
	// -------------------------
	settingsGroup->setLayout(settingsLayout);
	mainLayout->addWidget(settingsGroup);
	// -------------------------
	// Settings group
	// -------------------------

	setCentralWidget(centralWidget);

	adjustSize();
	setFixedSize(size());

	QScreen* screen = QApplication::screenAt(QCursor::pos());
	if (screen) {
		QRect screenGeometry = screen->availableGeometry();
		int x				 = (screenGeometry.width() - width()) / 2 + screenGeometry.x();
		int y				 = (screenGeometry.height() - height()) / 2 + screenGeometry.y();
		move(x, y);
	}

	eventBus.onBattery([this](bool onBattery) {
		this->onBattery = onBattery;
		onBatteryEvent();
	});

	eventBus.onGameEvent([this](size_t runningGames) {
		this->runningGames = runningGames;
		onGameEvent();
	});
}

void MainWindow::closeEvent(QCloseEvent* event) {
	event->ignore();
	this->hide();
}

void MainWindow::onGameEvent() {
	_profileDropdown->setEnabled(runningGames == 0 && !onBattery);
	_schedulerDropdown->setEnabled(runningGames == 0);
}

void MainWindow::onBatteryEvent() {
	_profileDropdown->setEnabled(runningGames == 0 && !onBattery);
}

void MainWindow::onBootSoundEvent(bool value) {
	_bootSoundDropdown->setCurrentIndex(_bootSoundDropdown->findData(value));
}

void MainWindow::setPerformanceProfile(PerformanceProfile value) {
	_profileDropdown->setCurrentIndex(_profileDropdown->findData(toString(value).c_str()));
}

void MainWindow::setScheduler(std::optional<std::string> sched) {
	_schedulerDropdown->setCurrentIndex(_schedulerDropdown->findData(sched.value_or("").c_str()));
}

void MainWindow::setBatteryChargeLimit(BatteryThreshold value) {
	_thresholdDropdown->setCurrentIndex(_thresholdDropdown->findData(toInt(value)));
}

void MainWindow::setAuraBrightness(RgbBrightness brightness) {
	_brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(toInt(brightness)));
}

void MainWindow::setAuraColor(std::optional<std::string> color) {
	_colorButton->setStyleSheet(("background-color: " + (color.value_or(Color::Black.toHex()))).c_str());
	_colorButton->setEnabled(color.has_value());
}

void MainWindow::setAuraEffect(std::string effect) {
	_effectDropdown->setCurrentIndex(_effectDropdown->findData(effect.c_str()));
}

void MainWindow::onProfileChanged(int) {
	PerformanceProfile profile = fromString<PerformanceProfile>(_profileDropdown->currentData().toString().toStdString());
	if (steamService.getRunningGames().empty()) {
		if (performanceService.getPerformanceProfile() != profile) {
			performanceService.setPerformanceProfile(profile);
		}
	}
}

void MainWindow::onSchedulerChanged(int) {
	std::optional<std::string> scheduler = (_schedulerDropdown->currentData().toString().toStdString());
	if (scheduler.value_or("").empty()) {
		scheduler = std::nullopt;
	}

	if (scheduler != performanceService.getCurrentScheduler() && steamService.getRunningGames().empty()) {
		performanceService.setScheduler(scheduler);
	}
}

void MainWindow::onBatteryLimitChanged(int) {
	auto threshold = fromInt<BatteryThreshold>(_thresholdDropdown->currentData().toInt());
	if (hardwareService.getChargeThreshold() != threshold) {
		hardwareService.setChargeThreshold(threshold);
	}
}

void MainWindow::showColorPicker() {
	QColor initial = QColor(openRgbService.getColor().value().c_str());
	QColor chosen  = QColorDialog::getColor(initial, this, translator.translate("pick.color").c_str());

	if (chosen.isValid()) {
		openRgbService.setColor(chosen.name(QColor::HexRgb).toStdString());
	}
}

void MainWindow::onEffectChange() {
	std::string effect = openRgbService.getAvailableEffects()[_effectDropdown->currentIndex()];
	if (openRgbService.getCurrentEffect() != effect) {
		openRgbService.setEffect(effect);
	}
}

void MainWindow::onBrightnessChange(int) {
	auto level = fromInt<RgbBrightness>(_brightnessDropdown->currentData().toInt());
	if (openRgbService.getCurrentBrightness() != level) {
		openRgbService.setBrightness(level);
	}
}

void MainWindow::openGameList() {
	GameList* list = new GameList(this);
	list->show();
}

void MainWindow::openFanEditor() {
	auto profile		= _profileDropdown->currentData().toString().toStdString();
	CurveEditor* editor = new CurveEditor(profile, this);
	editor->show();
}

void MainWindow::onAutostartChanged(bool enabled) {
	applicationService.setAutostart(enabled);
}

void MainWindow::onStartMinimizedChanged(bool enabled) {
	applicationService.setStartMinimized(enabled);
}

void MainWindow::onBootSoundChanged(int) {
	auto enabled = _bootSoundDropdown->currentData().toBool();
	if (hardwareService.getBootSound() != enabled) {
		hardwareService.setBootSound(enabled);
	}
}