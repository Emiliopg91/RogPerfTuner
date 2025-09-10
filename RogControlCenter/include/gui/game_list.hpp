#pragma once

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

#include "../services/hardware_service.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"

class GameList : public QDialog {
	Q_OBJECT
  public:
	inline static GameList* INSTANCE = nullptr;

	GameList(QWidget* parent = nullptr, bool manage_parent = false);

  protected:
	void closeEvent(QCloseEvent* event) override;

  private:
	QWidget* parentWidget = nullptr;
	bool manageParent	  = false;

	HardwareService& hardwareService = HardwareService::getInstance();
	SteamService& steamService		 = SteamService::getInstance();
	Translator& translator			 = Translator::getInstance();
};
