#include "../../include/gui/yes_no_dialog.hpp"

#include <qmessagebox.h>

bool YesNoDialog::showDialog(std::string title, std::string body, QWidget* parent) {
	QMessageBox::StandardButton reply = QMessageBox::question(parent, title.c_str(), body.c_str(), QMessageBox::Yes | QMessageBox::No);
	return reply == QMessageBox::Yes;
}