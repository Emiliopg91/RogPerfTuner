#include "../../include/gui/tray_icon.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"

#include "RccCommons.hpp"

#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QActionGroup>

TrayIcon::TrayIcon()
{
    tray_icon_.setIcon(QIcon::fromTheme(Constants::ICON_FILE.c_str()));
    tray_icon_.setToolTip("RogControlCenter");

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
        QObject::connect(act, &QAction::triggered, [this, bct]()
                         { this->onBatteryLimitChanged(bct); });
        chargeLimitMenu->addAction(act);
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
        QObject::connect(act, &QAction::triggered, [this, item]()
                         { this->onEffectChanged(item); });
        effectMenu->addAction(act);
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
        QObject::connect(act, &QAction::triggered, [this, item]()
                         { this->onBrightnessChanged(item); });
        brightnessMenu->addAction(act);
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
        QObject::connect(act, &QAction::triggered, [this, item]()
                         { this->onPerformanceProfileChanged(item); });
        profileMenu->addAction(act);
    }
    menu->insertMenu(nullptr, profileMenu);
    // -------------------------
    // Profile submenu
    // -------------------------
    // -------------------------
    // Performance menu
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
}

void TrayIcon::onBatteryLimitChanged(BatteryThreshold value)
{
    HardwareService::getInstance().setChargeThreshold(value);
}

void TrayIcon::onPerformanceProfileChanged(PerformanceProfile value)
{
    ProfileService::getInstance().setPerformanceProfile(value);
}

void TrayIcon::onEffectChanged(std::string effect)
{
    OpenRgbService::getInstance().setEffect(effect);
}

void TrayIcon::onBrightnessChanged(RgbBrightness brightness)
{
    OpenRgbService::getInstance().setBrightness(brightness);
}