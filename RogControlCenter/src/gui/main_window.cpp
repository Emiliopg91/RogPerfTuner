#include "../../include/gui/main_window.hpp"

#include <QApplication>
#include <algorithm>

#include "../../include/events/event_bus.hpp"
#include "../../include/gui/game_list.hpp"
#include "../../include/services/application_service.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), _logger(new Logger()) {
	setWindowTitle(QString::fromStdString(Constants::APP_NAME + " v" + Constants::APP_VERSION));
	setGeometry(0, 0, 350, 680);
	setFixedSize(350, 680);
	setWindowIcon(QIcon(QString::fromStdString(Constants::ICON_45_FILE)));

	QWidget* centralWidget	= new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->setAlignment(Qt::AlignTop);

	// Imagen
	QLabel* imageLabel = new QLabel();
	QPixmap pixmap(QString::fromStdString(Constants::ICON_FILE));
	QPixmap scaledPixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	imageLabel->setPixmap(scaledPixmap);
	imageLabel->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(imageLabel, 0, Qt::AlignCenter);

	// Grupo performance
	QGroupBox* performanceGroup	   = new QGroupBox(QString::fromStdString(translator.translate("performance")));
	QFormLayout* performanceLayout = new QFormLayout();
	performanceLayout->setContentsMargins(20, 20, 20, 20);

	_profileDropdown = new QComboBox();
	_profileDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items = PerformanceProfile::getAll();
	std::reverse(items.begin(), items.end());
	for (PerformanceProfile item : items) {
		_profileDropdown->addItem(QString::fromStdString(translator.translate("label.profile." + item.toName())), item.toInt());
	}
	connect(_profileDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);
	setPerformanceProfile(profileService.getPerformanceProfile());
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("profile") + ":")), _profileDropdown);

	_gameProfileButton = new QPushButton(QString::fromStdString(translator.translate("label.game.configure")));
	_gameProfileButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(_gameProfileButton, &QPushButton::clicked, this, &MainWindow::openGameList);
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("games") + ":")), _gameProfileButton);

	performanceGroup->setLayout(performanceLayout);
	mainLayout->addWidget(performanceGroup);

	// Grupo Aura
	QGroupBox* auraGroup	= new QGroupBox("Aura");
	QFormLayout* auraLayout = new QFormLayout();
	auraLayout->setContentsMargins(20, 20, 20, 20);

	_effectDropdown = new QComboBox();
	_effectDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto effects = openRgbService.getAvailableEffects();
	for (const auto effect : effects) {
		_effectDropdown->addItem(QString::fromStdString(effect), QString::fromStdString(effect));
	}

	auto it = std::find(effects.begin(), effects.end(), openRgbService.getCurrentEffect());
	_effectDropdown->setCurrentIndex(std::distance(effects.begin(), it));
	connect(_effectDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEffectChange);
	auraLayout->addRow(new QLabel(QString::fromStdString(translator.translate("effect") + ":")), _effectDropdown);

	_brightnessDropdown = new QComboBox();
	_brightnessDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto brightnesses = RgbBrightness::getAll();
	for (RgbBrightness b : brightnesses) {
		_brightnessDropdown->addItem(QString::fromStdString(translator.translate("label.brightness." + b.toName())), b.toInt());
	}
	_brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(openRgbService.getCurrentBrightness().toInt()));
	connect(_brightnessDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBrightnessChange);
	auraLayout->addRow(new QLabel(QString::fromStdString(translator.translate("brightness") + ":")), _brightnessDropdown);

	auraGroup->setLayout(auraLayout);
	mainLayout->addWidget(auraGroup);

	// Grupo Settings
	QGroupBox* settingsGroup	= new QGroupBox(QString::fromStdString(translator.translate("settings")));
	QFormLayout* settingsLayout = new QFormLayout();
	settingsLayout->setContentsMargins(20, 20, 20, 20);

	_thresholdDropdown = new QComboBox();
	_thresholdDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto thresholds = BatteryThreshold::getAll();
	for (BatteryThreshold t : thresholds) {
		_thresholdDropdown->addItem(QString::number(t.toInt()) + "%", t.toInt());
	}
	connect(_thresholdDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBatteryLimitChanged);
	setBatteryChargeLimit(hardwareService.getChargeThreshold());
	settingsLayout->addRow(new QLabel(QString::fromStdString(translator.translate("charge.threshold") + ":")), _thresholdDropdown);

	_autostart = new QCheckBox();
	_autostart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_autostart->setChecked(applicationService.isAutostartEnabled());
	connect(_autostart, &QCheckBox::toggled, this, &MainWindow::onAutostartChanged);
	settingsLayout->addRow(_autostart, new QLabel(QString::fromStdString(translator.translate("autostart"))));

	settingsGroup->setLayout(settingsLayout);
	mainLayout->addWidget(settingsGroup);

	setCentralWidget(centralWidget);
	QScreen* screen = QApplication::screenAt(QCursor::pos());
	if (screen) {
		QRect screenGeometry = screen->availableGeometry();
		int x				 = (screenGeometry.width() - width()) / 2 + screenGeometry.x();
		int y				 = (screenGeometry.height() - height()) / 2 + screenGeometry.y();
		move(x, y);
	}

	eventBus.onChargeThreshold([this](BatteryThreshold threshold) {
		setBatteryChargeLimit(threshold);
	});

	eventBus.onRgbBrightness([this](RgbBrightness brightness) {
		setAuraBrightness(brightness);
	});

	eventBus.onRgbEffect([this](std::string effect) {
		setAuraEffect(effect);
	});

	eventBus.onPerformanceProfile([this](PerformanceProfile profile) {
		setPerformanceProfile(profile);
	});

	eventBus.onGameEvent([this](size_t runningGames) {
		onGameEvent(runningGames);
	});
}

void MainWindow::closeEvent(QCloseEvent* event) {
	event->ignore();
	this->hide();
}

void MainWindow::onGameEvent(int runningGames) {
	_profileDropdown->setEnabled(runningGames == 0);
}

void MainWindow::setPerformanceProfile(PerformanceProfile value) {
	_profileDropdown->setCurrentIndex(_profileDropdown->findData(value.toInt()));
}

void MainWindow::setBatteryChargeLimit(BatteryThreshold value) {
	_thresholdDropdown->setCurrentIndex(_thresholdDropdown->findData(value.toInt()));
}

void MainWindow::setAuraBrightness(RgbBrightness brightness) {
	_brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(brightness.toInt()));
}

void MainWindow::setAuraEffect(std::string effect) {
	_effectDropdown->setCurrentIndex(_effectDropdown->findData(QString::fromStdString(effect)));
}

void MainWindow::onProfileChanged(int) {
	auto profile = PerformanceProfile::fromInt(_profileDropdown->currentData().toInt());
	if (steamService.getRunningGames().empty()) {
		if (profileService.getPerformanceProfile() != profile) {
			profileService.setPerformanceProfile(profile);
		}
	}
}

void MainWindow::onBatteryLimitChanged(int) {
	auto threshold = BatteryThreshold::fromInt(_thresholdDropdown->currentData().toInt());
	if (hardwareService.getChargeThreshold() != threshold) {
		hardwareService.setChargeThreshold(threshold);
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

void MainWindow::onAutostartChanged(bool enabled) {
	applicationService.setAutostart(enabled);
}