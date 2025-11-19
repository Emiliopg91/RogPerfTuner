#include "../../include/gui/main_window.hpp"

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

#include "../../include/gui/changelog_view.hpp"
#include "../../include/gui/fan_curve_editor.hpp"
#include "../../include/gui/game_list.hpp"
#include "../../include/utils/string_utils.hpp"
#include "OpenRGB/Color.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), _logger(new Logger()) {
	onBattery	 = uPowerClient.isOnBattery();
	runningGames = steamService.getRunningGames().size();

	setWindowTitle(QString::fromStdString(Constants::APP_NAME));
	setWindowIcon(QIcon(QString::fromStdString(Constants::ASSET_ICON_45_FILE)));

	QWidget* centralWidget	= new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
	mainLayout->setContentsMargins(20, 10, 20, 10);
	mainLayout->setAlignment(Qt::AlignTop);

	// -------------------------
	// Logo
	// -------------------------
	QLabel* imageLabel = new QLabel();
	QPixmap pixmap(QString::fromStdString(Constants::ASSET_ICON_FILE));
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
	QGroupBox* performanceGroup	   = new QGroupBox(QString::fromStdString(translator.translate("performance")));
	QFormLayout* performanceLayout = new QFormLayout();
	performanceLayout->setContentsMargins(20, 10, 20, 10);
	// -------------------------
	// Profile menu
	// -------------------------
	_profileDropdown = new QComboBox();
	_profileDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items = PerformanceProfile::getAll();
	std::reverse(items.begin(), items.end());
	for (PerformanceProfile item : items) {
		_profileDropdown->addItem(QString::fromStdString("  " + translator.translate("label.profile." + item.toName())),
								  QString::fromStdString(item.toString()));
	}
	connect(_profileDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);
	setPerformanceProfile(performanceService.getPerformanceProfile());
	_profileDropdown->setEnabled(runningGames == 0 && !onBattery);
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("profile") + ":")), _profileDropdown);

	eventBus.onPerformanceProfile([this](PerformanceProfile profile) {
		setPerformanceProfile(profile);
	});
	// -------------------------
	// Profile menu
	// -------------------------
	// -------------------------
	// Scheduler menu
	// -------------------------
	if (!performanceService.getAvailableSchedulers().empty()) {
		_schedulerDropdown = new QComboBox();
		_schedulerDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		_schedulerDropdown->addItem(QString::fromStdString("  " + translator.translate("label.scheduler.none")), QString(""));

		auto items2 = performanceService.getAvailableSchedulers();
		std::reverse(items2.begin(), items2.end());
		for (auto item : items2) {
			_schedulerDropdown->addItem("  " + QString::fromStdString(StringUtils::capitalize(item)), QString::fromStdString(item));
		}
		connect(_schedulerDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSchedulerChanged);
		setScheduler(performanceService.getCurrentScheduler());
		_schedulerDropdown->setEnabled(runningGames == 0);
		performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("scheduler") + ":")), _schedulerDropdown);

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
		QPushButton* fanEdit = new QPushButton();
		fanEdit->setText(QString::fromStdString(translator.translate("edit.curve")));
		connect(fanEdit, &QPushButton::clicked, this, &MainWindow::openFanEditor);
		fanEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("fan.curves") + ":")), fanEdit);
	}
	// -------------------------
	// Fan curves menu
	// -------------------------
	// -------------------------
	// Games menu
	// -------------------------
	_gameProfileButton = new QPushButton(QString::fromStdString(translator.translate("label.game.configure")));
	_gameProfileButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(_gameProfileButton, &QPushButton::clicked, this, &MainWindow::openGameList);
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("games") + ":")), _gameProfileButton);
	// -------------------------
	// Games menu
	// -------------------------
	performanceGroup->setLayout(performanceLayout);
	mainLayout->addWidget(performanceGroup);
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
	_effectDropdown = new QComboBox();
	_effectDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto effects = openRgbService.getAvailableEffects();
	for (const auto effect : effects) {
		_effectDropdown->addItem("  " + QString::fromStdString(effect), QString::fromStdString(effect));
	}

	auto it = std::find(effects.begin(), effects.end(), openRgbService.getCurrentEffect());
	_effectDropdown->setCurrentIndex(std::distance(effects.begin(), it));
	connect(_effectDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEffectChange);
	auraLayout->addRow(new QLabel(QString::fromStdString(translator.translate("effect") + ":")), _effectDropdown);

	eventBus.onRgbEffect([this](std::string effect) {
		setAuraEffect(effect);
	});
	// -------------------------
	// Effect menu
	// -------------------------
	// -------------------------
	// Brightness + Color menu
	// -------------------------
	_brightnessDropdown = new QComboBox();
	_brightnessDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto brightnesses = RgbBrightness::getAll();
	for (RgbBrightness b : brightnesses) {
		_brightnessDropdown->addItem("  " + QString::fromStdString(translator.translate("label.brightness." + b.toName())), b.toInt());
	}
	_brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(openRgbService.getCurrentBrightness().toInt()));
	connect(_brightnessDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBrightnessChange);

	_colorButton = new QPushButton();
	_colorButton->setFixedSize(25, 25);
	_colorButton->setStyleSheet(QString::fromStdString("background-color: " + (openRgbService.getColor().value_or(Color::Black.toHex()))));
	_colorButton->setCursor(Qt::PointingHandCursor);
	_colorButton->setEnabled(openRgbService.getColor().has_value());
	connect(_colorButton, &QPushButton::clicked, this, &MainWindow::showColorPicker);

	auto brightnessColorLayout = new QHBoxLayout();
	brightnessColorLayout->addWidget(_brightnessDropdown);
	brightnessColorLayout->addWidget(_colorButton);
	brightnessColorLayout->setSpacing(10);

	auraLayout->addRow(new QLabel(QString::fromStdString(translator.translate("brightness") + ":")), brightnessColorLayout);

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
		QGroupBox* hardwareGroup	= new QGroupBox(QString::fromStdString(translator.translate("hardware")));
		QFormLayout* hardwareLayout = new QFormLayout();
		hardwareLayout->setContentsMargins(20, 10, 20, 10);
		// -------------------------
		// Battery menu
		// -------------------------
		if (hardwareService.getBatteryLimitAvailable()) {
			_thresholdDropdown = new QComboBox();
			_thresholdDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			auto thresholds = BatteryThreshold::getAll();
			for (BatteryThreshold t : thresholds) {
				_thresholdDropdown->addItem("  " + QString::number(t.toInt()) + "%", t.toInt());
			}
			connect(_thresholdDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBatteryLimitChanged);
			setBatteryChargeLimit(hardwareService.getChargeThreshold());
			hardwareLayout->addRow(new QLabel(QString::fromStdString(translator.translate("charge.threshold") + ":")), _thresholdDropdown);

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
			_bootSoundDropdown = new QComboBox();
			_bootSoundDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

			auto vals = std::array<bool, 2>{true, false};
			for (size_t i = 0; i < vals.size(); i++) {
				_bootSoundDropdown->addItem(("  " + translator.translate(vals[i] ? "enabled" : "disabled")).c_str(), vals[i]);
			}
			onBootSoundEvent(hardwareService.getBootSound());

			connect(_bootSoundDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBootSoundChanged);
			hardwareLayout->addRow(new QLabel(QString::fromStdString(translator.translate("boot.sound"))), _bootSoundDropdown);

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
	QGroupBox* settingsGroup	= new QGroupBox(QString::fromStdString(translator.translate("settings")));
	QFormLayout* settingsLayout = new QFormLayout();
	settingsLayout->setContentsMargins(20, 10, 20, 10);
	// -------------------------
	// Autostart menu
	// -------------------------
	_autostart = new QCheckBox();
	_autostart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
#ifdef DEV_MODE
	_autostart->setEnabled(false);
#endif
	_autostart->setChecked(applicationService.isAutostartEnabled());
	connect(_autostart, &QCheckBox::toggled, this, &MainWindow::onAutostartChanged);
	settingsLayout->addRow(_autostart, new QLabel(QString::fromStdString(translator.translate("autostart"))));
	// -------------------------
	// Autostart menu
	// -------------------------
	settingsGroup->setLayout(settingsLayout);
	mainLayout->addWidget(settingsGroup);
	// -------------------------
	// Settings group
	// -------------------------

	// -------------------------
	// Status bar
	// -------------------------
	statusBar	 = new QStatusBar(this);
	versionLabel = new QLabel(Constants::APP_VERSION.c_str());
	statusBar->addPermanentWidget(versionLabel);
	setStatusBar(statusBar);
	// -------------------------
	// Status bar
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

#ifdef AUR_HELPER
	eventBus.onUpdateAvailable([this](std::string value) {
		onUpdateAvailable(value);
	});

	eventBus.onUpdateStart([this]() {
		onUpdateStart();
	});
#endif
}

#ifdef AUR_HELPER
void MainWindow::onUpdateAvailable(std::string value) {
	versionLabel->setText("");

	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			updateButton = new QPushButton(translator.translate("update.application", {{"version", value}}).c_str());
			updateButton->setFlat(true);
			updateButton->setStyleSheet("QPushButton {"
										"  padding: 0;"
										"  margin: 0;"
										"  border: none;"
										"  background: transparent;"
										"}"
										"QPushButton:hover {"
										"  border: none;"
										"  background: transparent;"
										"}"
										"QPushButton:focus {"
										"  outline: none;"
										"  border: none;"
										"}"
										"QPushButton:pressed {"
										"  border: none;"
										"  background: transparent;"
										"}");
			updateButton->setCursor(Qt::PointingHandCursor);
			statusBar->addPermanentWidget(updateButton);

			QObject::connect(updateButton, &QPushButton::clicked, [this]() {
				auto* changelog = new ChangelogView(this);
				changelog->setAttribute(Qt::WA_DeleteOnClose);
				changelog->show();
			});
		},
		Qt::QueuedConnection);
}

void MainWindow::onUpdateStart() {
	versionLabel->setText("");

	QMetaObject::invokeMethod(
		this,
		[=, this]() {
			versionLabel->setText(translator.translate("update.in.progress").c_str());
			statusBar->removeWidget(updateButton);
		},
		Qt::QueuedConnection);
}
#endif

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
	_profileDropdown->setCurrentIndex(_profileDropdown->findData(QString::fromStdString(value.toString())));
}

void MainWindow::setScheduler(std::optional<std::string> sched) {
	_schedulerDropdown->setCurrentIndex(_schedulerDropdown->findData(QString::fromStdString(sched.value_or(""))));
}

void MainWindow::setBatteryChargeLimit(BatteryThreshold value) {
	_thresholdDropdown->setCurrentIndex(_thresholdDropdown->findData(value.toInt()));
}

void MainWindow::setAuraBrightness(RgbBrightness brightness) {
	_brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(brightness.toInt()));
}

void MainWindow::setAuraColor(std::optional<std::string> color) {
	_colorButton->setStyleSheet(QString::fromStdString("background-color: " + (color.value_or(Color::Black.toHex()))));
	_colorButton->setEnabled(color.has_value());
}

void MainWindow::setAuraEffect(std::string effect) {
	_effectDropdown->setCurrentIndex(_effectDropdown->findData(QString::fromStdString(effect)));
}

void MainWindow::onProfileChanged(int) {
	PerformanceProfile profile = PerformanceProfile::fromString(_profileDropdown->currentData().toString().toStdString());
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
	auto threshold = BatteryThreshold::fromInt(_thresholdDropdown->currentData().toInt());
	if (hardwareService.getChargeThreshold() != threshold) {
		hardwareService.setChargeThreshold(threshold);
	}
}

void MainWindow::showColorPicker() {
	QColor initial = QColor(QString::fromStdString(openRgbService.getColor().value()));
	QColor chosen  = QColorDialog::getColor(initial, this, QString::fromStdString(translator.translate("pick.color")));

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
	auto level = RgbBrightness::fromInt(_brightnessDropdown->currentData().toInt());
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

void MainWindow::onBootSoundChanged(int) {
	auto enabled = _bootSoundDropdown->currentData().toBool();
	if (hardwareService.getBootSound() != enabled) {
		hardwareService.setBootSound(enabled);
	}
}