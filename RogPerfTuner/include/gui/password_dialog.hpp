#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "utils/configuration/configuration.hpp"
#include "utils/translator/translator.hpp"

class PasswordDialog : public QDialog, public Singleton<PasswordDialog> {
	Q_OBJECT

  public:
	bool showDialog();

  private:
	friend class Singleton<PasswordDialog>;
	PasswordDialog(QWidget* parent = nullptr);

	void onAccept();

	bool checkPassword(const QString& password);

	QLabel* label_;
	QLineEdit* password_input_;
	QPushButton* ok_button_;
	QPushButton* cancel_button_;

	Translator& translator		 = Translator::getInstance();
	Configuration& configuration = Configuration::getInstance();
};
