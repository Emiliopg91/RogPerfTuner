#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "../configuration/configuration.hpp"
#include "../logger/logger.hpp"
#include "../services/hardware_service.hpp"
#include "../services/steam_service.hpp"
#include "../shell/shell.hpp"
#include "../translator/translator.hpp"
#include "../utils/gui_utils.hpp"

class GameConfigDialog : public QDialog {
	Q_OBJECT

  public:
	void showDialog();
	GameConfigDialog(Logger logger, unsigned int gid, QWidget* parent = nullptr);

  private:
	void onAccept();

	bool checkPassword(const QString& password);

	QFormLayout* windowLayout;
	QGroupBox* group	= new QGroupBox(QString());
	QFormLayout* layout = new QFormLayout();

	NoScrollComboBox* gpuCombo;
	NoScrollComboBox* modeCombo;
	NoScrollComboBox* metricsCombo;
	NoScrollComboBox* wineSyncCombo;
	QLineEdit* envInput;
	QLineEdit* paramsInput;

	QPushButton* save_button_;

	unsigned int gid;
	GameEntry gameEntry;
	Logger logger;

	Translator& translator			 = Translator::getInstance();
	Configuration& configuration	 = Configuration::getInstance();
	HardwareService& hardwareService = HardwareService::getInstance();
	SteamService& steamService		 = SteamService::getInstance();
	Shell& shell					 = Shell::getInstance();
};
