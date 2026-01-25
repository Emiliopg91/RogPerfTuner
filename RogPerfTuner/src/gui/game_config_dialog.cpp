#include "../../include/gui/game_config_dialog.hpp"

#include <qcontainerfwd.h>

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <optional>

#include "../../include/gui/yes_no_dialog.hpp"
#include "../../include/utils/string_utils.hpp"

GameConfigDialog::GameConfigDialog(unsigned int gid, bool runAfterSave, QWidget* parent)
	: Loggable("GameConfigDialog"), QDialog(parent), gid(gid), runAfterSave(runAfterSave) {
	Logger::add_tab();
	gameEntry = configuration.getConfiguration().games[std::to_string(gid)];
	setWindowTitle(gameEntry.name.c_str());

	windowLayout = new QFormLayout();
	windowLayout->setAlignment(Qt::AlignTop);

	QVBoxLayout* headerLayout = new QVBoxLayout();
	headerLayout->setAlignment(Qt::AlignCenter);

	auto iconPath = steamService.getIcon(gid);
	if (iconPath.has_value()) {
		setWindowIcon(QIcon(iconPath.value().c_str()));
	}

	auto bannerPath = steamService.getBanner(gid);
	if (bannerPath.has_value()) {
		QPixmap gameIcon(QString::fromStdString(*bannerPath));
		int targetSize	   = 300;
		QPixmap scaledIcon = gameIcon.scaled(targetSize, targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		QLabel* imageLabel = new QLabel();
		imageLabel->setPixmap(scaledIcon);
		imageLabel->setAlignment(Qt::AlignCenter);

		headerLayout->addWidget(imageLabel);
	}
	windowLayout->addRow(headerLayout);
	group  = new QGroupBox();
	layout = new QFormLayout();

	int index, i;

	// --- PERFORMANCE ---
	QGroupBox* performanceGroup	   = new QGroupBox(QString::fromStdString(translator.translate("performance")));
	QFormLayout* performanceLayout = new QFormLayout();
	performanceLayout->setContentsMargins(20, 10, 20, 10);

	// --- GPU ---
	gpuCombo = new NoScrollComboBox();
	gpuCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	index = 0;
	i	  = 1;

	gpuCombo->addItem(QString::fromStdString(translator.translate("label.dgpu.auto")), QString::fromStdString(""));
	auto gpus = hardwareService.getGpus();
	for (const auto& [key, val] : gpus) {
		gpuCombo->addItem(QString::fromStdString(StringUtils::capitalize(key)), QString::fromStdString(key));
		if (gameEntry.gpu.has_value() && gameEntry.gpu.value() == key) {
			index = i;
		}
		i++;
	}
	gpuCombo->setCurrentIndex(index);
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("used.gpu") + ":")), gpuCombo);
	// --- GPU ---

	// --- Scheduler ---
	schedulerCombo = new NoScrollComboBox();
	schedulerCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	index = 0;
	i	  = 1;

	schedulerCombo->addItem(QString::fromStdString(translator.translate("label.scheduler.none")), QString::fromStdString(""));
	auto scheds = performanceService.getAvailableSchedulers();
	for (const auto& sched : scheds) {
		schedulerCombo->addItem(QString::fromStdString(StringUtils::capitalize(sched)), QString::fromStdString(sched));
		if (gameEntry.scheduler.has_value() && gameEntry.scheduler.value() == sched) {
			index = i;
		}
		i++;
	}
	schedulerCombo->setCurrentIndex(index);
	schedulerCombo->setEnabled(!performanceService.getAvailableSchedulers().empty());
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("scheduler") + ":")), schedulerCombo);
	// --- Scheduler ---

	// --- Metrics ---
	metricsCombo = new NoScrollComboBox();
	metricsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items = MangoHudLevelNS::values();
	i		   = 0;
	for (MangoHudLevel level : items) {
		metricsCombo->addItem(QString::fromStdString(translator.translate("label.level." + MangoHudLevelNS::toString(level))),
							  QString::fromStdString(MangoHudLevelNS::toString(level)));
		if (level == gameEntry.metrics_level) {
			metricsCombo->setCurrentIndex(i);
		}
		i++;
	}
	metricsCombo->setEnabled(steamService.metricsEnabled());
	performanceLayout->addRow(new QLabel(QString::fromStdString(translator.translate("metrics") + ":")), metricsCombo);
	// --- Metrics ---

	performanceGroup->setLayout(performanceLayout);
	windowLayout->addWidget(performanceGroup);
	// --- PERFORMANCE ---

	// --- PROTON ---
	if (gameEntry.proton) {
		QGroupBox* protonGroup	  = new QGroupBox("Proton");
		QFormLayout* protonLayout = new QFormLayout();
		protonLayout->setContentsMargins(20, 10, 20, 10);

		// --- Device ---
		i		  = 0;
		modeCombo = new NoScrollComboBox();
		modeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		auto items2 = ComputerTypeNS::values();
		for (ComputerType opt : items2) {
			modeCombo->addItem(QString::fromStdString(translator.translate("label.device." + ComputerTypeNS::toString(opt))),
							   QString::fromStdString(ComputerTypeNS::toString(opt)));
			if (opt == gameEntry.device) {
				modeCombo->setCurrentIndex(i);
			}

			i++;
		}
		protonLayout->addRow(new QLabel(QString::fromStdString(translator.translate("used.device") + ":")), modeCombo);
		// --- Device ---

		// --- Wine Sync ---
		i			  = 0;
		wineSyncCombo = new NoScrollComboBox();
		wineSyncCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		auto items3 = WineSyncOptionNS::values();
		for (WineSyncOption opt : items3) {
			wineSyncCombo->addItem(QString::fromStdString(translator.translate("label.winesync." + WineSyncOptionNS::toString(opt))),
								   QString::fromStdString(WineSyncOptionNS::toString(opt)));
			if (opt == gameEntry.sync) {
				wineSyncCombo->setCurrentIndex(i);
			}

			i++;
		}
		protonLayout->addRow(new QLabel(QString::fromStdString(translator.translate("winesync") + ":")), wineSyncCombo);
		// --- Wine Sync ---

		protonGroup->setLayout(protonLayout);
		windowLayout->addWidget(protonGroup);
	}
	// --- PROTON ---

	// --- Execution ---
	QGroupBox* executionGroup	 = new QGroupBox(translator.translate("execution").c_str());
	QFormLayout* executionLayout = new QFormLayout();
	executionLayout->setContentsMargins(20, 10, 20, 10);

	// --- Environment ---
	envInput = new QLineEdit(QString::fromStdString(gameEntry.env.value_or("")));
	executionLayout->addRow(new QLabel(QString::fromStdString(translator.translate("environment") + ":")), envInput);
	// --- Environment ---

	// --- Wrappers ---
	wrappersInput = new QLineEdit(QString::fromStdString(gameEntry.wrappers.value_or("")));
	executionLayout->addRow(new QLabel(QString::fromStdString(translator.translate("wrappers") + ":")), wrappersInput);
	// --- Wrappers ---

	// --- Parameters ---
	paramsInput = new QLineEdit(QString::fromStdString(gameEntry.args.value_or("")));
	executionLayout->addRow(new QLabel(QString::fromStdString(translator.translate("params") + ":")), paramsInput);
	// --- Parameters ---

	executionGroup->setLayout(executionLayout);
	windowLayout->addWidget(executionGroup);
	// --- EXECUTION ---

	// --- Save button ---
	save_button_ = new QPushButton(QString::fromStdString(translator.translate(runAfterSave ? "save.and.run" : "save")));
	save_button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	windowLayout->addRow(save_button_);
	connect(save_button_, &QPushButton::clicked, this, &GameConfigDialog::onAccept);

	setLayout(windowLayout);

	adjustSize();
	setFixedSize(size());
}

void GameConfigDialog::onAccept() {
	logger.info("Accepted configuration");
	std::optional<std::string> gpu = gpuCombo->currentData().toString().toStdString();
	if (gpu.value().empty()) {
		gpu = std::nullopt;
	}
	std::optional<std::string> scheduler = schedulerCombo->currentData().toString().toStdString();
	if (scheduler.value().empty()) {
		scheduler = std::nullopt;
	}

	MangoHudLevel level = MangoHudLevel::NO_DISPLAY;
	if (steamService.metricsEnabled()) {
		level = MangoHudLevelNS::fromString(metricsCombo->currentData().toString().toStdString());
	}

	WineSyncOption sync = WineSyncOption::AUTO;
	ComputerType device = ComputerType::COMPUTER;
	if (gameEntry.proton) {
		device = ComputerTypeNS::fromString(modeCombo->currentData().toString().toStdString());
		sync   = WineSyncOptionNS::fromString(wineSyncCombo->currentData().toString().toStdString());
	}

	gameEntry.args			= paramsInput->text().toStdString();
	gameEntry.env			= envInput->text().toStdString();
	gameEntry.wrappers		= wrappersInput->text().toStdString();
	gameEntry.scheduler		= scheduler;
	gameEntry.gpu			= gpu;
	gameEntry.metrics_level = level;
	gameEntry.device		= device;
	gameEntry.sync			= sync;

	steamService.saveGameConfig(gid, gameEntry);

	accept();
}

void GameConfigDialog::showDialog() {
	exec();

	if (runAfterSave) {
		steamService.launchGame(gameEntry.overlayId.value_or(std::to_string(gid)));
	}
}

void GameConfigDialog::closeEvent(QCloseEvent* event) {
	if (runAfterSave) {
		auto reply = YesNoDialog::showDialog(translator.translate("confirmation.required"), translator.translate("run.with.default.config"), this);
		if (reply) {
			logger.info("Game will launch with default configuration");
			event->accept();
		} else {
			event->ignore();
		}
	}
}