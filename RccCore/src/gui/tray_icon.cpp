#include "../../include/configuration/configuration.hpp"
#include "../../include/gui/game_list.hpp"
#include "../../include/gui/main_window.hpp"
#include "../../include/gui/tray_icon.hpp"
#include "../../include/models/battery_charge_threshold.hpp"
#include "../../include/models/performance_profile.hpp"
#include "../../include/models/rgb_brightness.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/events.hpp"
#include "../../include/utils/event_bus.hpp"

#include "RccCommons.hpp"

#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QActionGroup>

void openSettings()
{
    Shell::getInstance().run_command("xdg-open " + Constants::CONFIG_FILE);
}

void reloadSettings()
{
    Configuration::getInstance().loadConfig();
}

void openLogs()
{
    Shell::getInstance().run_command("xdg-open " + Constants::LOG_DIR + "/" + Constants::LOG_FILE_NAME + ".log");
}

void openGameList()
{
    if (!GameList::INSTANCE)
        if (MainWindow::INSTANCE)
            GameList::INSTANCE = new GameList(MainWindow::INSTANCE);
        else
            GameList::INSTANCE = new GameList();
    GameList::INSTANCE->show();
}

void openMainWindow()
{
    if (!MainWindow::INSTANCE)
        MainWindow::INSTANCE = new MainWindow();

    MainWindow::INSTANCE->show();
}

void TrayIcon::setAuraBrightness(RgbBrightness brightness)
{
    brightnessActions[brightness.toName()]->setChecked(true);
}

void TrayIcon::setAuraEffect(std::string effect)
{
    effectActions[effect]->setChecked(true);
}

void TrayIcon::setPerformanceProfile(PerformanceProfile profile)
{
    perfProfileActions[profile.toName()]->setChecked(true);
}

void TrayIcon::setBatteryThreshold(BatteryThreshold threshold)
{
    thresholdActions[threshold.toName()]->setChecked(true);
}

void onBatteryLimitChanged(BatteryThreshold value)
{
    HardwareService::getInstance().setChargeThreshold(value);
}

void onPerformanceProfileChanged(PerformanceProfile value)
{
    ProfileService::getInstance().setPerformanceProfile(value);
}

void onEffectChanged(std::string effect)
{
    OpenRgbService::getInstance().setEffect(effect);
}

void onBrightnessChanged(RgbBrightness brightness)
{
    OpenRgbService::getInstance().setBrightness(brightness);
}

TrayIcon::TrayIcon()
{
    tray_icon_.setIcon(QIcon::fromTheme(Constants::ASSET_ICON_FILE.c_str()));
    tray_icon_.setToolTip(QString::fromStdString(Constants::APP_NAME + " v" + Constants::APP_VERSION));

    auto translator = Translator::getInstance();

    QMenu *menu = new QMenu();

    // -------------------------
    // Battery menu
    // -------------------------
    QAction *batteryTitle = new QAction(translator.translate("battery").c_str(), menu);
    batteryTitle->setEnabled(false);
    menu->addAction(batteryTitle);
    // -------------------------
    // BatteryThreshold submenu
    // -------------------------
    QMenu *chargeLimitMenu = new QMenu(("    " + translator.translate("charge.threshold")).c_str(), menu);
    QActionGroup *chargeGroup = new QActionGroup(menu);
    for (BatteryThreshold bct : BatteryThreshold::getAll())
    {
        QAction *act = new QAction((std::to_string(bct.toInt()) + "%").c_str(), chargeGroup);
        act->setCheckable(true);
        act->setChecked(bct == HardwareService::getInstance().getChargeThreshold());
        QObject::connect(act, &QAction::triggered, [bct]()
                         { onBatteryLimitChanged(bct); });
        chargeLimitMenu->addAction(act);
        thresholdActions[bct.toName()] = act;
    }
    menu->insertMenu(nullptr, chargeLimitMenu);
    // -------------------------
    // BatteryThreshold submenu
    // -------------------------
    // -------------------------
    // Battery menu
    // -------------------------

    menu->addSeparator();

    // -------------------------
    // Aura menu
    // -------------------------
    QAction *auraTitle = new QAction("AuraSync", menu);
    auraTitle->setEnabled(false);
    menu->addAction(auraTitle);
    // -------------------------
    // Effect submenu
    // -------------------------
    QMenu *effectMenu = new QMenu(("    " + translator.translate("effect")).c_str(), menu);
    QActionGroup *effectGroup = new QActionGroup(menu);
    auto effects = OpenRgbService::getInstance().getAvailableEffects();
    for (std::string item : effects)
    {
        QAction *act = new QAction(item.c_str(), effectGroup);
        act->setCheckable(true);
        act->setChecked(item == OpenRgbService::getInstance().getCurrentEffect());
        QObject::connect(act, &QAction::triggered, [item]()
                         { onEffectChanged(item); });
        effectMenu->addAction(act);
        effectActions[item] = act;
    }
    menu->insertMenu(nullptr, effectMenu);
    // -------------------------
    // Effect submenu
    // -------------------------
    // -------------------------
    // Brightness submenu
    // -------------------------
    QMenu *brightnessMenu = new QMenu(("    " + translator.translate("brightness")).c_str(), menu);
    QActionGroup *brightnessGroup = new QActionGroup(menu);
    auto levels = RgbBrightness::getAll();
    for (RgbBrightness item : levels)
    {
        QAction *act = new QAction(
            Translator::getInstance().translate("label.brightness." + item.toName()).c_str(), brightnessGroup);
        act->setCheckable(true);
        act->setChecked(item == OpenRgbService::getInstance().getCurrentBrightness());
        QObject::connect(act, &QAction::triggered, [item]()
                         { onBrightnessChanged(item); });
        brightnessMenu->addAction(act);
        brightnessActions[item.toName()] = act;
    }
    menu->insertMenu(nullptr, brightnessMenu);
    // -------------------------
    // Brightness submenu
    // -------------------------
    // -------------------------
    // Aura menu
    // -------------------------

    menu->addSeparator();

    // -------------------------
    // Performance menu
    // -------------------------
    QAction *performanceTitle = new QAction(translator.translate("performance").c_str(), menu);
    performanceTitle->setEnabled(false);
    menu->addAction(performanceTitle);
    // -------------------------
    // Profile submenu
    // -------------------------
    QMenu *profileMenu = new QMenu(("    " + translator.translate("profile")).c_str(), menu);
    QActionGroup *profileGroup = new QActionGroup(menu);
    auto items = PerformanceProfile::getAll();
    std::reverse(items.begin(), items.end());
    for (PerformanceProfile item : items)
    {
        QAction *act = new QAction(translator.translate("label.profile." + item.toName()).c_str(), profileGroup);
        act->setCheckable(true);
        act->setChecked(item == ProfileService::getInstance().getPerformanceProfile());
        QObject::connect(act, &QAction::triggered, [item]()
                         { onPerformanceProfileChanged(item); });
        profileMenu->addAction(act);
        perfProfileActions[item.toName()] = act;
    }
    menu->insertMenu(nullptr, profileMenu);
    // -------------------------
    // Profile submenu
    // -------------------------
    // -------------------------
    // Game submenu
    // -------------------------
    QMenu *gamesMenu = new QMenu(("    " + translator.translate("games")).c_str(), menu);
    menu->insertMenu(nullptr, gamesMenu);

    QAction *act = new QAction((translator.translate("label.game.configure") + "...").c_str());
    QObject::connect(act, &QAction::triggered, []()
                     { openGameList(); });
    gamesMenu->addAction(act);
    // -------------------------
    // Game submenu
    // -------------------------
    // -------------------------
    // Performance menu
    // -------------------------

    menu->addSeparator();

    if (Constants::DEV_MODE)
    {
        // -------------------------
        // Develop submenu
        // -------------------------
        QAction *developTitle = new QAction("Development", menu);
        developTitle->setEnabled(false);
        menu->addAction(developTitle);
        // -------------------------
        // Settings submenu
        // -------------------------
        QMenu *settingsMenu = new QMenu("    Settings", menu);
        menu->insertMenu(nullptr, settingsMenu);

        QAction *openSettingsAct = new QAction("Open settings");
        QObject::connect(openSettingsAct, &QAction::triggered, []()
                         { openSettings(); });
        settingsMenu->addAction(openSettingsAct);

        QAction *reloadSettingsAct = new QAction("Reload settings");
        QObject::connect(reloadSettingsAct, &QAction::triggered, []()
                         { reloadSettings(); });
        settingsMenu->addAction(reloadSettingsAct);
        // -------------------------
        // Settings submenu
        // -------------------------
        // -------------------------
        // Logs submenu
        // -------------------------
        QMenu *logsMenu = new QMenu("    Logs", menu);
        menu->insertMenu(nullptr, logsMenu);

        QAction *openLogsAct = new QAction("Open logs");
        QObject::connect(openLogsAct, &QAction::triggered, []()
                         { openLogs(); });
        logsMenu->addAction(openLogsAct);
        // -------------------------
        // Logs submenu
        // -------------------------
        // -------------------------
        // Develop menu
        // -------------------------
    }

    menu->addSeparator();

    // -------------------------
    // Main window
    // -------------------------

    QAction *openAction = new QAction(translator.translate("open.ui").c_str(), menu);
    QObject::connect(openAction, &QAction::triggered, []()
                     { openMainWindow(); });
    menu->addAction(openAction);

    // -------------------------
    // Main window
    // -------------------------

    menu->addSeparator();

    // -------------------------
    // Exit
    // -------------------------
    QAction *quitAction = new QAction(translator.translate("close").c_str(), menu);
    QObject::connect(quitAction, &QAction::triggered, []()
                     { exit(0); });
    menu->addAction(quitAction);
    // -------------------------
    // Exit
    // -------------------------

    tray_icon_.setContextMenu(menu);

    tray_icon_.show();

    EventBus::getInstance().on_with_data(Events::ORGB_SERVICE_ON_BRIGHTNESS, [this](CallbackParam data)
                                         { setAuraBrightness(std::any_cast<RgbBrightness>(data[0])); });

    EventBus::getInstance().on_with_data(Events::ORGB_SERVICE_ON_EFFECT, [this](CallbackParam data)
                                         { setAuraEffect(std::any_cast<std::string>(data[0])); });

    EventBus::getInstance().on_with_data(Events::PROFILE_SERVICE_ON_PROFILE, [this](CallbackParam data)
                                         { setPerformanceProfile(std::any_cast<PerformanceProfile>(data[0])); });

    EventBus::getInstance().on_with_data(Events::HARDWARE_SERVICE_THRESHOLD_CHANGED, [this](CallbackParam data)
                                         { setBatteryThreshold(std::any_cast<BatteryThreshold>(data[0])); });

    EventBus::getInstance().on_with_data(Events::STEAM_SERVICE_GAME_EVENT, [this, profileMenu](CallbackParam data)
                                         { profileMenu->setEnabled(std::any_cast<size_t>(data[0]) == 0); });
}