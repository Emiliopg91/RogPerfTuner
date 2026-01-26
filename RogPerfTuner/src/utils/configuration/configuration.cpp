#include "../../../include/utils/configuration/configuration.hpp"

#include <qobject.h>
#include <qt6keychain/keychain.h>

#include <QEventLoop>
#include <fstream>
#include <string>

#include "../../../include/utils/constants.hpp"
#include "../../../include/utils/file_utils.hpp"
#include "../../../include/utils/logger/logger_provider.hpp"

void Configuration::loadConfig() {
	if (FileUtils::exists(Constants::CONFIG_FILE)) {
		logger.debug("Loading settings from '" + Constants::CONFIG_FILE + "'");
		try {
			auto node = YAML::LoadFile(Constants::CONFIG_FILE);
			config	  = node.as<RootConfig>();
			LoggerProvider::setConfigMap(config->logger);
		} catch (const std::exception& e) {
			logger.error("Error loading settings: " + std::string(e.what()));
		}
	} else {
		logger.debug("Settings file not found, creating new");
		FileUtils::createDirectory(Constants::CONFIG_DIR);

		RootConfig defaultCfg;
		config = defaultCfg;

		saveConfig();
	}
	logger.info("Configuration loaded");
}

void Configuration::saveConfig() {
	if (!config.has_value()) {
		return;
	}

	logger.debug("Configuration saved");

	try {
		YAML::Node node = YAML::convert<RootConfig>::encode(*config);

		std::ofstream fout(Constants::CONFIG_FILE);
		fout << node;
		fout.close();
	} catch (const std::exception& e) {
		logger.error("Error saving settings file: " + std::string(e.what()));
	}
}

const QString service = Constants::EXEC_NAME.c_str();
const QString key	  = "password";

std::string Configuration::getPassword() {
	QKeychain::ReadPasswordJob job(service);
	job.setKey(key);

	QEventLoop loop;
	QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
	job.start();
	loop.exec();

	if (job.error()) {
		logger.error("Error on password fetching: " + job.errorString().toStdString());
		return "";
	}

	return job.textData().toStdString();
}

void Configuration::setPassword(const std::string& pss) {
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

RootConfig& Configuration::getConfiguration() {
	if (!config.has_value()) {
		config = RootConfig{};
	}
	return config.value();
}

Configuration::Configuration() : Loggable("Configuration") {
	loadConfig();
}