#include "../../include/gui/game_config_dialog.hpp"

#include <qcontainerfwd.h>

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <optional>

#include "../../include/gui/yes_no_dialog.hpp"
#include "../../include/utils/file_utils.hpp"
#include "../../include/utils/string_utils.hpp"

GameConfigDialog::GameConfigDialog(unsigned int gid, bool runAfterSave, QWidget* parent)
	: Loggable("GameConfigDialog"), QDialog(parent), gid(gid), runAfterSave(runAfterSave) {
	Logger::add_tab();
	setWindowTitle(QString::fromStdString(translator.translate("config.for.game", {{"game", ""}})));
	setFixedSize(400, 400);

	gameEntry = configuration.getConfiguration().games[std::to_string(gid)];

	windowLayout = new QFormLayout();
	windowLayout->setAlignment(Qt::AlignTop);

	QVBoxLayout* headerLayout = new QVBoxLayout();
	headerLayout->setAlignment(Qt::AlignCenter);

	/*auto userIds = FileUtils::listDirectory(Constants::STEAM_USERDATA_PATH);
	for (const auto& userId : userIds) {
		auto path = fmt::format("{}/{}/config/grid/{}_hero.png", Constants::STEAM_USERDATA_PATH, userId, gid);
		if (FileUtils::exists(path)) {
			QPixmap gameIcon(QString::fromStdString(path));
			int targetSize	   = 300;
			QPixmap scaledIcon = gameIcon.scaled(targetSize, targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			QLabel* imageLabel = new QLabel();
			imageLabel->setPixmap(scaledIcon);
			imageLabel->setAlignment(Qt::AlignCenter);

			headerLayout->addWidget(imageLabel);
			setFixedSize(400, 500);
			break;
		}
	}*/

	QLabel* titleLabel = new QLabel(QString::fromStdString(gameEntry.name));
	QFont titleFont	   = titleLabel->font();
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleFont.setBold(true);
	titleLabel->setFont(titleFont);
	titleLabel->setAlignment(Qt::AlignCenter);

	headerLayout->addWidget(titleLabel);

	windowLayout->addRow(headerLayout);

	group  = new QGroupBox();
	layout = new QFormLayout();

	int index, i;

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
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("used.gpu") + ":")), gpuCombo);

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
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("scheduler") + ":")), schedulerCombo);

	// --- Metrics ---
	metricsCombo = new NoScrollComboBox();
	metricsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items = MangoHudLevel::getAll();
	i		   = 0;
	for (MangoHudLevel level : items) {
		metricsCombo->addItem(QString::fromStdString(translator.translate("label.level." + level.toString())),
							  QString::fromStdString(level.toString()));
		if (level == gameEntry.metrics_level) {
			metricsCombo->setCurrentIndex(i);
		}
		i++;
	}
	metricsCombo->setEnabled(steamService.metricsEnabled());
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("metrics") + ":")), metricsCombo);

	// --- Mode ---
	modeCombo = new NoScrollComboBox();
	modeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	modeCombo->addItem(QString::fromStdString(translator.translate("label.steamdeck.no")), false);
	modeCombo->addItem(QString::fromStdString(translator.translate("label.steamdeck.yes")), true);
	modeCombo->setCurrentIndex(gameEntry.steamdeck ? 1 : 0);
	modeCombo->setEnabled(gameEntry.proton);
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("used.steamdeck") + ":")), modeCombo);

	// --- Wine Sync ---

	i = 0;

	wineSyncCombo = new NoScrollComboBox();
	wineSyncCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	auto items2 = WineSyncOption::getAll();
	for (WineSyncOption opt : items2) {
		wineSyncCombo->addItem(QString::fromStdString(translator.translate("label.winesync." + opt.toString())),
							   QString::fromStdString(opt.toString()));
		if (opt == gameEntry.sync) {
			wineSyncCombo->setCurrentIndex(i);
		}

		i++;
	}
	wineSyncCombo->setEnabled(gameEntry.proton);
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("winesync") + ":")), wineSyncCombo);

	// --- Environment ---
	envInput = new QLineEdit(QString::fromStdString(gameEntry.env.value_or("")));
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("environment") + ":")), envInput);

	// --- Wrappers ---
	wrappersInput = new QLineEdit(QString::fromStdString(gameEntry.wrappers.value_or("")));
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("wrappers") + ":")), wrappersInput);

	// --- Parameters ---
	paramsInput = new QLineEdit(QString::fromStdString(gameEntry.args.value_or("")));
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("params") + ":")), paramsInput);

	// --- Save button ---
	save_button_ = new QPushButton(QString::fromStdString(translator.translate(runAfterSave ? "save.and.run" : "save")));
	save_button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	layout->addRow(save_button_);
	connect(save_button_, &QPushButton::clicked, this, &GameConfigDialog::onAccept);

	group->setLayout(layout);
	windowLayout->addWidget(group);
	setLayout(windowLayout);
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

	MangoHudLevel level = MangoHudLevel::Enum::NO_DISPLAY;
	if (steamService.metricsEnabled()) {
		level = MangoHudLevel::fromString(metricsCombo->currentData().toString().toStdString());
	}

	WineSyncOption sync = WineSyncOption::Enum::AUTO;
	if (gameEntry.proton) {
		sync = WineSyncOption::fromString(wineSyncCombo->currentData().toString().toStdString());
	}

	gameEntry.args			= paramsInput->text().toStdString();
	gameEntry.env			= envInput->text().toStdString();
	gameEntry.wrappers		= wrappersInput->text().toStdString();
	gameEntry.scheduler		= scheduler;
	gameEntry.gpu			= gpu;
	gameEntry.metrics_level = level.toString();
	gameEntry.steamdeck		= modeCombo->currentData().toBool();
	gameEntry.sync			= sync.toString();

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