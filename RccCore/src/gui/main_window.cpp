
#include <algorithm>

#include "../../include/gui/main_window.hpp"
#include "../../include/gui/game_list.hpp"
#include "../../include/services/application_service.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _logger(new Logger())
{
    setWindowTitle("RogControlCenter");
    setGeometry(0, 0, 350, 630);
    setFixedSize(350, 630);
    setWindowIcon(QIcon(QString::fromStdString(Constants::ICON_45_FILE)));

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setAlignment(Qt::AlignTop);

    // Imagen
    QLabel *imageLabel = new QLabel();
    QPixmap pixmap(QString::fromStdString(Constants::ICON_FILE));
    QPixmap scaledPixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaledPixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(imageLabel, 0, Qt::AlignCenter);

    // Grupo performance
    QGroupBox *performanceGroup = new QGroupBox(QString::fromStdString(Translator::getInstance().translate("performance")));
    QFormLayout *performanceLayout = new QFormLayout();
    performanceLayout->setContentsMargins(20, 20, 20, 20);

    _profileDropdown = new QComboBox();
    auto items = PerformanceProfile::getAll();
    std::reverse(items.begin(), items.end());
    for (PerformanceProfile item : items)
    {
        _profileDropdown->addItem(
            QString::fromStdString(Translator::getInstance().translate("label.profile." + item.toName())),
            item.toInt());
    }
    connect(_profileDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onProfileChanged);
    setPerformanceProfile(ProfileService::getInstance().getPerformanceProfile());
    performanceLayout->addRow(new QLabel(QString::fromStdString(Translator::getInstance().translate("profile") + ":")),
                              _profileDropdown);

    _gameProfileButton = new QPushButton(QString::fromStdString(Translator::getInstance().translate("label.game.configure")));
    connect(_gameProfileButton, &QPushButton::clicked, this, &MainWindow::openGameList);
    performanceLayout->addRow(new QLabel(QString::fromStdString(Translator::getInstance().translate("games") + ":")),
                              _gameProfileButton);

    performanceGroup->setLayout(performanceLayout);
    mainLayout->addWidget(performanceGroup);

    // Grupo Aura
    QGroupBox *auraGroup = new QGroupBox("Aura");
    QFormLayout *auraLayout = new QFormLayout();
    auraLayout->setContentsMargins(20, 20, 20, 20);

    _effectDropdown = new QComboBox();
    auto effects = OpenRgbService::getInstance().getAvailableEffects();
    for (const auto effect : effects)
        _effectDropdown->addItem(QString::fromStdString(effect), QString::fromStdString(effect));

    auto it = std::find(effects.begin(), effects.end(), OpenRgbService::getInstance().getCurrentEffect());
    _effectDropdown->setCurrentIndex(std::distance(effects.begin(), it));
    connect(_effectDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onEffectChange);
    auraLayout->addRow(new QLabel(QString::fromStdString(Translator::getInstance().translate("effect") + ":")),
                       _effectDropdown);

    _brightnessDropdown = new QComboBox();
    auto brightnesses = RgbBrightness::getAll();
    for (RgbBrightness b : brightnesses)
    {
        _brightnessDropdown->addItem(
            QString::fromStdString(Translator::getInstance().translate("label.brightness." + b.toName())),
            b.toInt());
    }
    _brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(OpenRgbService::getInstance().getCurrentBrightness().toInt()));
    connect(_brightnessDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onBrightnessChange);
    auraLayout->addRow(new QLabel(QString::fromStdString(Translator::getInstance().translate("brightness") + ":")),
                       _brightnessDropdown);

    auraGroup->setLayout(auraLayout);
    mainLayout->addWidget(auraGroup);

    // Grupo Battery
    QGroupBox *settingsGroup = new QGroupBox(QString::fromStdString(Translator::getInstance().translate("battery")));
    QFormLayout *settingsLayout = new QFormLayout();
    settingsLayout->setContentsMargins(20, 20, 20, 20);

    _thresholdDropdown = new QComboBox();
    auto thresholds = BatteryThreshold::getAll();
    for (BatteryThreshold t : thresholds)
        _thresholdDropdown->addItem(QString::number(t.toInt()) + "%", t.toInt());
    connect(_thresholdDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onBatteryLimitChanged);
    setBatteryChargeLimit(HardwareService::getInstance().getChargeThreshold());
    settingsLayout->addRow(new QLabel(QString::fromStdString(Translator::getInstance().translate("charge.threshold") + ":")),
                           _thresholdDropdown);

    settingsGroup->setLayout(settingsLayout);
    mainLayout->addWidget(settingsGroup);

    setCentralWidget(centralWidget);
    GuiUtils::centerWindowOnCurrentScreen(this);

    EventBus::getInstance().on_with_data(Events::HARDWARE_SERVICE_THRESHOLD_CHANGED, [this](CallbackParam v)
                                         { setBatteryChargeLimit(std::any_cast<BatteryThreshold>(v[0])); });

    EventBus::getInstance().on_with_data(Events::ORGB_SERVICE_ON_BRIGHTNESS, [this](CallbackParam data)
                                         { setAuraBrightness(std::any_cast<RgbBrightness>(data[0])); });

    EventBus::getInstance().on_with_data(Events::ORGB_SERVICE_ON_EFFECT, [this](CallbackParam data)
                                         { setAuraEffect(std::any_cast<std::string>(data[0])); });

    EventBus::getInstance().on_with_data(Events::PROFILE_SERVICE_ON_PROFILE, [this](CallbackParam data)
                                         { setPerformanceProfile(std::any_cast<PerformanceProfile>(data[0])); });

    EventBus::getInstance().on_with_data(Events::STEAM_SERVICE_GAME_EVENT, [this](CallbackParam data)
                                         { onGameEvent(std::any_cast<int>(data[0])); });
    /*
    EVENT_BUS.on(Events::PROFILE_SERVICE_ON_PROFILE, [this](PerformanceProfile v)
                 { setPerformanceProfile(v); });
    EVENT_BUS.on(Events::ORGB_SERVICE_ON_BRIGHTNESS, [this](auto v)
                 { setAuraState(v); });
    EVENT_BUS.on(Events::STEAM_SERVICE_GAME_EVENT, [this](int running)
                 { onGameEvent(running); });
*/
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
}

void MainWindow::onGameEvent(int runningGames)
{
    _profileDropdown->setEnabled(runningGames == 0);
}

void MainWindow::setPerformanceProfile(PerformanceProfile value)
{
    _profileDropdown->setCurrentIndex(_profileDropdown->findData(value.toInt()));
}

void MainWindow::setBatteryChargeLimit(BatteryThreshold value)
{
    _thresholdDropdown->setCurrentIndex(_thresholdDropdown->findData(value.toInt()));
}

void MainWindow::setAuraBrightness(RgbBrightness brightness)
{
    _brightnessDropdown->setCurrentIndex(_brightnessDropdown->findData(brightness.toInt()));
}

void MainWindow::setAuraEffect(std::string effect)
{
    _effectDropdown->setCurrentIndex(_effectDropdown->findData(QString::fromStdString(effect)));
}

void MainWindow::onProfileChanged(int index)
{
    auto profile = PerformanceProfile::fromInt(_profileDropdown->currentData().toInt());
    if (SteamService::getInstance().getRunningGames().empty())
    {
        if (ProfileService::getInstance().getPerformanceProfile() != profile)
        {
            ProfileService::getInstance().setPerformanceProfile(profile);
        }
    }
}

void MainWindow::onBatteryLimitChanged(int index)
{
    auto threshold = BatteryThreshold::fromInt(_thresholdDropdown->currentData().toInt());
    if (HardwareService::getInstance().getChargeThreshold() != threshold)
    {
        HardwareService::getInstance().setChargeThreshold(threshold);
    }
}

void MainWindow::onEffectChange()
{
    std::string effect = OpenRgbService::getInstance().getAvailableEffects()[_effectDropdown->currentIndex()];
    if (OpenRgbService::getInstance().getCurrentEffect() != effect)
    {
        OpenRgbService::getInstance().setEffect(effect);
    }
}

void MainWindow::onBrightnessChange(int index)
{
    auto level = RgbBrightness::fromInt(_brightnessDropdown->currentData().toInt());
    if (OpenRgbService::getInstance().getCurrentBrightness() != level)
    {
        OpenRgbService::getInstance().setBrightness(level);
    }
}

void MainWindow::openGameList()
{
    GameList *list = new GameList(this);
    list->show();
}