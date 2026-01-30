#include "utils/configuration_wrapper.hpp"

#include <qobject.h>
#include <qt6keychain/keychain.h>

#include <QEventLoop>
#include <string>

const QString service = Constants::EXEC_NAME.c_str();
const QString key	  = "password";

std::string ConfigurationWrapper::getPassword() {
	QKeychain::ReadPasswordJob job(service);
	job.setKey(key);

	QEventLoop loop;
	QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
	job.start();
	loop.exec();

	if (job.error()) {
		logger->error("Error on password fetching: {}", job.errorString().toStdString());
		return "";
	}

	return job.textData().toStdString();
}

void ConfigurationWrapper::setPassword(const std::string& pss) {
	QKeychain::WritePasswordJob job(service);
	job.setKey(key);
	job.setTextData(pss.c_str());

	QEventLoop loop;
	QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
	job.start();
	loop.exec();

	if (job.error()) {
		std::string msg = "Error saving password: " + job.errorString().toStdString();
		throw std::runtime_error(msg);
	}
}

ConfigurationWrapper::ConfigurationWrapper() : Configuration<RootConfig>(Constants::CONFIG_DIR, Constants::CONFIG_FILE) {
	loadConfig();
}