#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>

#include "framework/gui/no_scroll_combo_box.hpp"
#include "framework/shell/shell.hpp"
#include "framework/translator/translator.hpp"
#include "services/hardware_service.hpp"
#include "services/steam_service.hpp"
#include "utils/configuration_wrapper.hpp"

class GameConfigDialog : public QDialog, Loggable {
	Q_OBJECT

  public:
	void showDialog();
	GameConfigDialog(unsigned int gid, bool onGameFirstRun, QWidget* parent = nullptr);

  private:
	void onAccept();

	bool checkPassword(const QString& password);

	QFormLayout* windowLayout;
	QGroupBox* group	= new QGroupBox(QString());
	QFormLayout* layout = new QFormLayout();

	NoScrollComboBox* gpuCombo;
	NoScrollComboBox* schedulerCombo;
	NoScrollComboBox* modeCombo;
	NoScrollComboBox* metricsCombo;
	NoScrollComboBox* wineSyncCombo;
	QLineEdit* envInput;
	QLineEdit* wrappersInput;
	QLineEdit* paramsInput;

	QPushButton* save_button_;

	unsigned int gid;
	GameEntry gameEntry;
	bool onGameFirstRun;

	Translator& translator				   = Translator::getInstance();
	ConfigurationWrapper& configuration	   = ConfigurationWrapper::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
	Shell& shell						   = Shell::getInstance();

  protected:
	void closeEvent(QCloseEvent*);
};
