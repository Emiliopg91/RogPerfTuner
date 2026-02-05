#include "gui/game_config_dialog.hpp"

#include <qcontainerfwd.h>

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <optional>

#include "framework/utils/string_utils.hpp"
#include "gui/yes_no_dialog.hpp"

GameConfigDialog::GameConfigDialog(unsigned int gid, bool runAfterSave, QWidget* parent)
	: Loggable("GameConfigDialog"), QDialog(parent), gid(gid), runAfterSave(runAfterSave) {
	Logger::add_tab();
	gameEntry = configuration.getConfiguration().games[gid];
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
		QPixmap gameIcon((*bannerPath).c_str());
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
	QGroupBox* performanceGroup	   = new QGroupBox(translator.translate("performance").c_str());
	QFormLayout* performanceLayout = new QFormLayout();
	performanceLayout->setContentsMargins(20, 10, 20, 10);

	// --- GPU ---
	gpuCombo = new NoScrollComboBox();
	gpuCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	index = 0;
	i	  = 1;

	gpuCombo->addItem(translator.translate("label.dgpu.auto").c_str(), "");
	auto gpus = hardwareService.getGpus();
	for (const auto& [key, val] : gpus) {
		gpuCombo->addItem(StringUtils::capitalize(key).c_str(), key.c_str());
		if (gameEntry.gpu.has_value() && gameEntry.gpu.value() == key) {
			index = i;
		}
		i++;
	}
	gpuCombo->setCurrentIndex(index);
	performanceLayout->addRow(new QLabel((translator.translate("used.gpu") + ":").c_str()), gpuCombo);
	// --- GPU ---

	// --- Scheduler ---
	schedulerCombo = new NoScrollComboBox();
	schedulerCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	index = 0;
	i	  = 1;

	schedulerCombo->addItem(translator.translate("label.scheduler.none").c_str(), "");
	auto scheds = performanceService.getAvailableSchedulers();
	for (const auto& sched : scheds) {
		schedulerCombo->addItem(StringUtils::capitalize(sched).c_str(), sched.c_str());
		if (gameEntry.scheduler.has_value() && gameEntry.scheduler.value() == sched) {
			index = i;
		}
		i++;
	}
	schedulerCombo->setCurrentIndex(index);
	schedulerCombo->setEnabled(!performanceService.getAvailableSchedulers().empty());
	performanceLayout->addRow(new QLabel((translator.translate("scheduler") + ":").c_str()), schedulerCombo);
	// --- Scheduler ---

	// --- Metrics ---
	metricsCombo = new NoScrollComboBox();
	metricsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items = values<MangoHudLevel>();
	i		   = 0;
	for (MangoHudLevel level : items) {
		metricsCombo->addItem(translator.translate("label.level." + toString(level)).c_str(), toString(level).c_str());
		if (level == gameEntry.metrics_level) {
			metricsCombo->setCurrentIndex(i);
		}
		i++;
	}
	metricsCombo->setEnabled(steamService.metricsEnabled());
	performanceLayout->addRow(new QLabel((translator.translate("metrics") + ":").c_str()), metricsCombo);
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
		auto items2 = values<ComputerType>();
		for (ComputerType opt : items2) {
			modeCombo->addItem(translator.translate("label.device." + toString(opt)).c_str(), toString(opt).c_str());
			if (opt == gameEntry.device) {
				modeCombo->setCurrentIndex(i);
			}

			i++;
		}
		protonLayout->addRow(new QLabel((translator.translate("used.device") + ":").c_str()), modeCombo);
		// --- Device ---

		// --- Wine Sync ---
		i			  = 0;
		wineSyncCombo = new NoScrollComboBox();
		wineSyncCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		auto items3 = values<WineSyncOption>();
		for (WineSyncOption opt : items3) {
			wineSyncCombo->addItem(translator.translate("label.winesync." + toString(opt)).c_str(), toString(opt).c_str());
			if (opt == gameEntry.sync) {
				wineSyncCombo->setCurrentIndex(i);
			}

			i++;
		}
		protonLayout->addRow(new QLabel((translator.translate("winesync") + ":").c_str()), wineSyncCombo);
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
	envInput = new QLineEdit(gameEntry.env.value_or("").c_str());
	executionLayout->addRow(new QLabel((translator.translate("environment") + ":").c_str()), envInput);
	// --- Environment ---

	// --- Wrappers ---
	wrappersInput = new QLineEdit(gameEntry.wrappers.value_or("").c_str());
	executionLayout->addRow(new QLabel((translator.translate("wrappers") + ":").c_str()), wrappersInput);
	// --- Wrappers ---

	// --- Parameters ---
	paramsInput = new QLineEdit(gameEntry.args.value_or("").c_str());
	executionLayout->addRow(new QLabel((translator.translate("params") + ":").c_str()), paramsInput);
	// --- Parameters ---

	executionGroup->setLayout(executionLayout);
	windowLayout->addWidget(executionGroup);
	// --- EXECUTION ---

	// --- Save button ---
	save_button_ = new QPushButton(translator.translate(runAfterSave ? "save.and.run" : "save").c_str());
	save_button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	windowLayout->addRow(save_button_);
	connect(save_button_, &QPushButton::clicked, this, &GameConfigDialog::onAccept);

	setLayout(windowLayout);

	adjustSize();
	setFixedSize(size());
}

void GameConfigDialog::onAccept() {
	logger->info("Accepted configuration");
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
		level = fromString<MangoHudLevel>(metricsCombo->currentData().toString().toStdString());
	}

	WineSyncOption sync = WineSyncOption::AUTO;
	ComputerType device = ComputerType::COMPUTER;
	if (gameEntry.proton) {
		device = fromString<ComputerType>(modeCombo->currentData().toString().toStdString());
		sync   = fromString<WineSyncOption>(wineSyncCombo->currentData().toString().toStdString());
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
			logger->info("Game will launch with default configuration");
			event->accept();
		} else {
			event->ignore();
		}
	}
}