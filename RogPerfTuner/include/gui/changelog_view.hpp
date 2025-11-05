#pragma once

#include <qpushbutton.h>

#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QTextBrowser>

#include "../services/application_service.hpp"

class ChangelogView : public QDialog {
	Q_OBJECT

  public:
	ChangelogView(QWidget* parent = nullptr);

  private:
	QTextBrowser* changelogViewer;
	QPushButton* updateButton;
	ApplicationService& applicationService = ApplicationService::init(std::nullopt);
	Translator& translator				   = Translator::getInstance();

  private slots:
	void onUpdateClicked();
	void closeEvent(QCloseEvent* event);
};
