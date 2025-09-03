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

#include "../models/hardware/gpu_brand.hpp"
#include "../models/steam/mangohud_level.hpp"
#include "../models/steam/wine_sync_option.hpp"
#include "../services/hardware_service.hpp"
#include "../services/steam_service.hpp"  // tu clase equivalente
#include "../translator/translator.hpp"
#include "../utils/gui_utils.hpp"
#include "RccCommons.hpp"

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
};
