#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "../translator/translator.hpp"

class PasswordDialog : public QDialog {
	Q_OBJECT

  public:
	static PasswordDialog& getInstance() {
		static PasswordDialog instance;
		return instance;
	}

	bool showDialog();

  private:
	PasswordDialog(QWidget* parent = nullptr);

	void onAccept();

	bool checkPassword(const QString& password);

	QLabel* label_;
	QLineEdit* password_input_;
	QPushButton* ok_button_;
	QPushButton* cancel_button_;

	Translator& translator = Translator::getInstance();
};
