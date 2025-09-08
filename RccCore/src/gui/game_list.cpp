#include "../../include/gui/game_list.hpp"

#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QPixmap>
#include <QScrollArea>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "../../include/models/hardware/gpu_brand.hpp"
#include "../../include/models/steam/mangohud_level.hpp"
#include "../../include/models/steam/wine_sync_option.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/steam_service.hpp"	 // tu clase equivalente
#include "../../include/translator/translator.hpp"
#include "../../include/utils/gui_utils.hpp"
#include "RccCommons.hpp"

GameList::GameList(QWidget* parent, bool manage_parent) : QDialog(parent), manageParent(manage_parent), parentWidget(parent) {
	if (!INSTANCE)
		INSTANCE = this;

	setWindowTitle(QString::fromStdString(translator.translate("game.performance.configuration")));
	setFixedSize(1000, 600);
	setWindowIcon(QIcon(QString::fromStdString(Constants::ICON_45_FILE)));
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* layout = new QVBoxLayout(this);

	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	QWidget* container = new QWidget();
	scrollArea->setWidget(container);

	QVBoxLayout* containerLayout = new QVBoxLayout(container);

	auto gameCfg = steamService.getGames();
	QVector<unsigned int> appIds;
	for (const auto& [key, val] : gameCfg)
		appIds.append(static_cast<unsigned int>(std::stoul(key)));

	auto metricsEnabled = steamService.metricsEnabled();

	QStringList columns;
	columns << QString::fromStdString(translator.translate("game.title")) << QString::fromStdString(translator.translate("used.gpu"))
			<< QString::fromStdString(translator.translate("used.steamdeck"));
	if (metricsEnabled)
		columns << QString::fromStdString(translator.translate("metrics"));
	columns << QString::fromStdString(translator.translate("winesync")) << QString::fromStdString(translator.translate("environment"))
			<< QString::fromStdString(translator.translate("params"));

	QTableWidget* table = new QTableWidget(appIds.size(), columns.size());
	table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	table->setHorizontalHeaderLabels(columns);
	table->verticalHeader()->setVisible(false);

	QHeaderView* header = table->horizontalHeader();
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i = 1; i < columns.size(); ++i)
		header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
	header->setHighlightSections(false);

	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionMode(QAbstractItemView::NoSelection);
	table->setFocusPolicy(Qt::NoFocus);

	int row = 0;
	std::sort(appIds.begin(), appIds.end(), [&](int a, int b) { return gameCfg[std::to_string(a)].name > gameCfg[std::to_string(b)].name; });

	for (unsigned int appid : appIds) {
		bool isRunning = steamService.isRunning(appid);

		// --- Title ---
		QTableWidgetItem* item = new QTableWidgetItem(
			QString::fromStdString(gameCfg[std::to_string(appid)].name + (isRunning ? " (" + translator.translate("running") + "...)" : "")));

		item->setFlags(Qt::ItemIsEnabled);
		item->setToolTip(QString::number(appid));
		if (isRunning)
			item->setForeground(QColor("green"));

		table->setItem(row, 0, item);
		// --- GPU ---
		NoScrollComboBox* gpuCombo = new NoScrollComboBox();
		gpuCombo->setEnabled(!isRunning);
		gpuCombo->addItem(QString::fromStdString(translator.translate("label.dgpu.auto")), QString::fromStdString(""));
		int gpuIndex = 0;
		auto gpus	 = hardwareService.getGpus();
		int i		 = 0;
		for (const auto& [key, val] : gpus) {
			gpuCombo->addItem(QString::fromStdString(StringUtils::capitalize(key)), QString::fromStdString(key));
			if (steamService.getPreferedGpu(appid).has_value() && key == steamService.getPreferedGpu(appid).value().toString())
				gpuIndex = i + 1;
			i++;
		}
		gpuCombo->setCurrentIndex(gpuIndex);

		connect(gpuCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int) {
			std::string str				  = gpuCombo->currentData().toString().toStdString();
			std::optional<GpuBrand> brand = std::nullopt;
			if (!str.empty())
				brand = GpuBrand::fromString(str);

			steamService.setPreferedGpu(appid, brand);
		});
		table->setCellWidget(row, 1, gpuCombo);

		// --- Steamdeck ---
		NoScrollComboBox* sdCombo = new NoScrollComboBox();
		sdCombo->setEnabled(!isRunning);
		sdCombo->addItem(QString::fromStdString(translator.translate("label.steamdeck.no")), false);
		sdCombo->addItem(QString::fromStdString(translator.translate("label.steamdeck.yes")), true);
		sdCombo->setCurrentIndex(steamService.isSteamDeck(appid) ? 1 : 0);

		connect(sdCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
				[=](int) { steamService.setSteamDeck(appid, sdCombo->currentData().toBool()); });
		table->setCellWidget(row, 2, sdCombo);

		// --- Metrics ---
		int col = 3;
		if (steamService.metricsEnabled()) {
			NoScrollComboBox* metricsCombo = new NoScrollComboBox();
			metricsCombo->setEnabled(!isRunning);
			auto items = MangoHudLevel::getAll();
			for (MangoHudLevel level : items) {
				metricsCombo->addItem(QString::fromStdString(translator.translate("label.level." + std::to_string(level.toInt()))), level.toInt());
				if (level == steamService.getMetricsLevel(appid))
					metricsCombo->setCurrentIndex(level.toInt());
			}

			connect(metricsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
					[=](int) { steamService.setMetricsLevel(appid, MangoHudLevel::fromInt(metricsCombo->currentData().toInt())); });
			table->setCellWidget(row, col++, metricsCombo);
		}

		// --- WineSync ---
		NoScrollComboBox* syncCombo = new NoScrollComboBox();
		auto items					= WineSyncOption::getAll();
		i							= 0;
		for (WineSyncOption opt : items) {
			syncCombo->addItem(QString::fromStdString(translator.translate("label.winesync." + opt.toString())),
							   QString::fromStdString(opt.toString()));
			if (opt == steamService.getWineSync(appid))
				syncCombo->setCurrentIndex(i);
			i++;
		}
		syncCombo->setEnabled(!isRunning && steamService.isProton(appid));
		connect(syncCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
				[=](int) { steamService.setWineSync(appid, syncCombo->currentData().toString().toStdString()); });
		table->setCellWidget(row, col++, syncCombo);

		// --- Environment ---
		QLineEdit* envInput = new QLineEdit(QString::fromStdString(steamService.getEnvironment(appid)));
		envInput->setEnabled(!isRunning);
		connect(envInput, &QLineEdit::textChanged, [=](const QString& text) { steamService.setEnvironment(appid, text.toStdString()); });
		table->setCellWidget(row, col++, envInput);

		// --- Parameters ---
		QLineEdit* paramsInput = new QLineEdit(QString::fromStdString(steamService.getParameters(appid)));
		paramsInput->setEnabled(!isRunning);
		connect(paramsInput, &QLineEdit::textChanged, [=](const QString& text) { steamService.setParameters(appid, text.toStdString()); });
		table->setCellWidget(row, col++, paramsInput);

		row++;
	}

	containerLayout->addWidget(table);
	layout->addWidget(scrollArea);
}

void GameList::closeEvent(QCloseEvent* event) {
	if (INSTANCE == this)
		INSTANCE = nullptr;
	QDialog::closeEvent(event);
}