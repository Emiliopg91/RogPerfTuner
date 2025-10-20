#include "../../include/gui/password_dialog.hpp"

#include <QMessageBox>
#include <QProcess>
#include <QVBoxLayout>

#include "../../include/configuration/configuration.hpp"
#include "../../include/translator/translator.hpp"

bool PasswordDialog::showDialog() {
	if (exec() == QDialog::Accepted) {
		return true;
	}

	QMessageBox::information(nullptr, "Canceled", QString::fromStdString(translator.translate("user.canceled.operation")));
	exit(EXIT_FAILURE);
}

PasswordDialog::PasswordDialog(QWidget* parent) : QDialog(parent) {
	setWindowTitle(QString::fromStdString(translator.translate("authentication.required")));
	setFixedSize(300, 150);

	QVBoxLayout* layout = new QVBoxLayout(this);

	label_ = new QLabel(QString::fromStdString(translator.translate("enter.sudo.password")));
	layout->addWidget(label_);

	password_input_ = new QLineEdit();
	password_input_->setEchoMode(QLineEdit::Password);
	layout->addWidget(password_input_);

	ok_button_ = new QPushButton(QString::fromStdString(translator.translate("accept")));
	layout->addWidget(ok_button_);
	connect(ok_button_, &QPushButton::clicked, this, &PasswordDialog::onAccept);

	cancel_button_ = new QPushButton(QString::fromStdString(translator.translate("cancel")));
	layout->addWidget(cancel_button_);
	connect(cancel_button_, &QPushButton::clicked, this, &PasswordDialog::reject);
}

void PasswordDialog::onAccept() {
	ok_button_->setDisabled(true);
	cancel_button_->setDisabled(true);

	if (checkPassword(password_input_->text())) {
		configuration.setPassword(password_input_->text().toStdString());
		configuration.saveConfig();
		accept();
	} else {
		ok_button_->setDisabled(false);
		cancel_button_->setDisabled(false);
		QMessageBox::warning(this, "Error", QString::fromStdString(translator.translate("authentication.failed")));
	}
}
bool PasswordDialog::checkPassword(const QString& password) {
	QProcess process;

	QStringList args;
	args << "-k" << "-S" << "echo" << "Password correct";

	process.start("sudo", args);

	if (!process.waitForStarted()) {
		return false;
	}

	// enviar contraseña
	process.write((password + "\n").toUtf8());
	process.closeWriteChannel();

	if (!process.waitForFinished()) {
		return false;
	}

	QByteArray stdoutData = process.readAllStandardOutput();
	QByteArray stderrData = process.readAllStandardError();
	int exitCode		  = process.exitCode();

	if (exitCode == 0 && stdoutData.contains("Password correct")) {
		qDebug() << "Authentication successful";
		return true;
	} else {
		qDebug() << "Authentication failed:" << stderrData;
		return false;
	}
}
