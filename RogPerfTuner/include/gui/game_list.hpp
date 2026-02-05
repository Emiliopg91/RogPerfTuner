#pragma once

#include <QDialog>

#include "framework/translator/translator.hpp"
#include "services/steam_service.hpp"

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

	Translator& translator	   = Translator::getInstance();
	SteamService& steamService = SteamService::getInstance();
};
