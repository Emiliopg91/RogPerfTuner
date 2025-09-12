#include "../../include/gui/game_config_dialog.hpp"

#include <qcontainerfwd.h>

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>
#include <optional>

void GameConfigDialog::showDialog() {
	if (exec() != QDialog::Accepted) {
		QMessageBox::information(nullptr, QString::fromStdString(translator.translate("configuration.skipped")),
								 QString::fromStdString(translator.translate("running.default")));
	}

	logger.info("Configuration finished");

	logger.info("Relaunching game with SteamOverlayId {}...", gameEntry.overlayId);
	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	shell.run_command("steam steam://rungameid/" + gameEntry.overlayId);

	Logger::rem_tab();
	Logger::rem_tab();
}

GameConfigDialog::GameConfigDialog(Logger logger, unsigned int gid, QWidget* parent) : QDialog(parent), gid(gid), logger(logger) {
	setWindowTitle(QString::fromStdString(translator.translate("config.for.game", {{"game", ""}})));
	setFixedWidth(400);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	adjustSize();
	setMaximumHeight(height());

	gameEntry = configuration.getConfiguration().games[std::to_string(gid)];

	windowLayout = new QFormLayout();
	windowLayout->setAlignment(Qt::AlignTop);

	group  = new QGroupBox(QString::fromStdString(gameEntry.name));
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

	// --- Mode ---
	modeCombo = new NoScrollComboBox();
	modeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	modeCombo->addItem(QString::fromStdString(translator.translate("label.steamdeck.no")), false);
	modeCombo->addItem(QString::fromStdString(translator.translate("label.steamdeck.yes")), true);
	modeCombo->setCurrentIndex(gameEntry.steamdeck ? 1 : 0);
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("used.steamdeck") + ":")), modeCombo);

	// --- Metrics ---
	if (steamService.metricsEnabled()) {
		metricsCombo = new NoScrollComboBox();
		metricsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		auto items = MangoHudLevel::getAll();
		for (MangoHudLevel level : items) {
			metricsCombo->addItem(QString::fromStdString(translator.translate("label.level." + std::to_string(level.toInt()))), level.toInt());
			if (level == gameEntry.metrics_level) {
				metricsCombo->setCurrentIndex(level.toInt());
			}
		}
		layout->addRow(new QLabel(QString::fromStdString(translator.translate("metrics") + ":")), metricsCombo);
	}

	if (gameEntry.proton) {
		// --- Wine Sync ---

		i = 0;

		wineSyncCombo = new NoScrollComboBox();
		wineSyncCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		auto items = WineSyncOption::getAll();
		for (WineSyncOption opt : items) {
			wineSyncCombo->addItem(QString::fromStdString(translator.translate("label.winesync." + opt.toString())),
								   QString::fromStdString(opt.toString()));
			if (opt == gameEntry.sync) {
				wineSyncCombo->setCurrentIndex(i);
			}

			i++;
		}
		layout->addRow(new QLabel(QString::fromStdString(translator.translate("winesync") + ":")), wineSyncCombo);
	}

	// --- Environment ---
	envInput = new QLineEdit(QString::fromStdString(gameEntry.env.value_or("")));
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("environment") + ":")), envInput);

	// --- Parameters ---
	paramsInput = new QLineEdit(QString::fromStdString(gameEntry.args.value_or("")));
	layout->addRow(new QLabel(QString::fromStdString(translator.translate("params") + ":")), paramsInput);

	// --- Save button ---
	save_button_ = new QPushButton(QString::fromStdString(translator.translate("save.and.run")));
	save_button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	layout->addRow(save_button_);
	connect(save_button_, &QPushButton::clicked, this, &GameConfigDialog::onAccept);

	group->setLayout(layout);
	windowLayout->addWidget(group);
	setLayout(windowLayout);
}

void GameConfigDialog::onAccept() {
	std::optional<std::string> gpu = gpuCombo->currentData().toString().toStdString();
	if (gpu.value().empty()) {
		gpu = std::nullopt;
	}

	MangoHudLevel level = MangoHudLevel::Enum::NO_DISPLAY;
	if (steamService.metricsEnabled()) {
		level = MangoHudLevel::fromInt(metricsCombo->currentData().toInt());
	}

	WineSyncOption sync = WineSyncOption::Enum::AUTO;
	if (gameEntry.proton) {
		sync = WineSyncOption::fromString(wineSyncCombo->currentData().toString().toStdString());
	}

	gameEntry.args			= paramsInput->text().toStdString();
	gameEntry.env			= envInput->text().toStdString();
	gameEntry.gpu			= gpu;
	gameEntry.metrics_level = level.toInt();
	gameEntry.steamdeck		= modeCombo->currentData().toBool();
	gameEntry.sync			= sync.toString();

	configuration.getConfiguration().games[std::to_string(gid)] = gameEntry;
	configuration.saveConfig();

	accept();
}