#include "../../include/configuration/configuration.hpp"

#include <qt6keychain/keychain.h>

#include <QEventLoop>
#include <fstream>

#include "../../include/logger/logger_provider.hpp"
#include "../../include/utils/constants.hpp"
#include "../../include/utils/file_utils.hpp"
#include "yaml-cpp/node/parse.h"

void Configuration::loadConfig() {
	if (FileUtils::exists(Constants::CONFIG_FILE)) {
		logger.debug("Loading settings from '{}'", Constants::CONFIG_FILE);
		try {
			auto node = YAML::LoadFile(Constants::CONFIG_FILE);
			config	  = node.as<RootConfig>();
			LoggerProvider::setConfigMap(config->logger);
		} catch (const std::exception& e) {
			logger.error("Error loading settings: {}", e.what());
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
		logger.error("Error saving settings file: '{}'", e.what());
	}
}

std::string Configuration::getPassword() {
	const QString service = "rog-perf-tuner";
	const QString key	  = "password";

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
	const QString service = "rog-perf-tuner";
	const QString key	  = "password";

	QKeychain::WritePasswordJob job(service);
	job.setKey(key);
	job.setTextData(QString::fromStdString(pss));

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