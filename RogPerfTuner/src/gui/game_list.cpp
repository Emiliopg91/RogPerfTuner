#include "../../include/gui/game_list.hpp"

#include <qcontainerfwd.h>
#include <qnamespace.h>

#include <QHeaderView>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QTableWidget>
#include <optional>

#include "../../include/gui/game_config_dialog.hpp"
#include "../../include/utils/constants.hpp"

GameList::GameList(QWidget* parent, bool manage_parent) : QDialog(parent), manageParent(manage_parent), parentWidget(parent) {
	if (!INSTANCE) {
		INSTANCE = this;
	}

	setWindowModality(Qt::WindowModal);

	setWindowTitle(translator.translate("game.performance.configuration").c_str());
	setFixedSize(500, 600);
	setWindowIcon(QIcon(Constants::ASSET_ICON_45_FILE.c_str()));
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
	for (const auto& [key, val] : gameCfg) {
		appIds.append(static_cast<unsigned int>(key));
	}

	auto metricsEnabled = steamService.metricsEnabled();

	QStringList columns;
	columns << translator.translate("game.title").c_str();
	columns << translator.translate("configuration").c_str();

	QTableWidget* table = new QTableWidget(appIds.size(), columns.size());
	table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	table->setHorizontalHeaderLabels(columns);
	table->verticalHeader()->setVisible(false);

	QHeaderView* header = table->horizontalHeader();
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	for (int i = 1; i < columns.size(); ++i) {
		header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
	}
	header->setHighlightSections(false);

	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setSelectionMode(QAbstractItemView::NoSelection);
	table->setFocusPolicy(Qt::NoFocus);

	int row = 0;
	std::sort(appIds.begin(), appIds.end(), [&](unsigned int a, unsigned int b) {
		const auto& na = gameCfg.at(a).name;  // usar at() para no insertar
		const auto& nb = gameCfg.at(b).name;
		return na < nb;	 // true si a debe ir antes que b (ascendente)
	});

	for (unsigned int appid : appIds) {
		bool isRunning = steamService.isRunning(appid);

		// --- Title ---
		QTableWidgetItem* item =
			new QTableWidgetItem((gameCfg[appid].name + (isRunning ? " (" + translator.translate("running") + "...)" : "")).c_str());

		auto iconPath = steamService.getIcon(appid);
		if (iconPath.has_value()) {
			item->setIcon(QIcon((*iconPath).c_str()));
		}

		item->setFlags(Qt::ItemIsEnabled);
		item->setToolTip(QString::number(appid));
		if (isRunning) {
			item->setForeground(QColor("green"));
		}

		table->setItem(row, 0, item);

		QPushButton* btn = new QPushButton(translator.translate("edit").c_str());
		btn->setProperty("appid", QVariant::fromValue<qulonglong>(appid));
		connect(btn, &QPushButton::clicked, this, [this, btn]() {
			unsigned int gid = btn->property("appid").toULongLong();
			GameConfigDialog(gid, false, this).showDialog();
		});

		table->setCellWidget(row, 1, btn);

		row++;
	}

	containerLayout->addWidget(table);
	layout->addWidget(scrollArea);
}

void GameList::closeEvent(QCloseEvent* event) {
	if (INSTANCE == this) {
		INSTANCE = nullptr;
	}
	QDialog::closeEvent(event);
}