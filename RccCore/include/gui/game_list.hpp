#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QLineEdit>
#include <QComboBox>
#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QString>
#include <QWidget>
#include <QCloseEvent>

#include "RccCommons.hpp"

#include "../services/steam_service.hpp" // tu clase equivalente
#include "../services/hardware_service.hpp"
#include "../translator/translator.hpp"
#include "../models/hardware/gpu_brand.hpp"
#include "../models/steam/mangohud_level.hpp"
#include "../models/steam/wine_sync_option.hpp"
#include "../utils/gui_utils.hpp"

class GameList : public QDialog
{
    Q_OBJECT
public:
    inline static GameList *INSTANCE = nullptr;

    GameList(QWidget *parent = nullptr, bool manage_parent = false);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QWidget *parentWidget = nullptr;
    bool manageParent = false;
};
