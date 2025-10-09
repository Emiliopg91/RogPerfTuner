#include "../../include/configuration/configuration.hpp"

#include <libsecret/secret.h>

#include <cstdlib>
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
	GError* error = nullptr;

	const SecretSchema schema = {
		"rog_control_center_password",	// name
		SECRET_SCHEMA_NONE,				// flags
		{								// attributes
		 {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
		 {NULL, (SecretSchemaAttributeType)0}},
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0  // reserved1..reserved7
	};

	gchar* password = secret_password_lookup_sync(&schema, nullptr, &error, "key", "default", nullptr);

	if (error) {
		std::string msg = "Error fetching password: " + std::string(error->message);
		g_error_free(error);
		throw new std::runtime_error(msg);
	}

	std::string result = password ? password : "";
	g_free(password);
	return result;
}

void Configuration::setPassword(const std::string& pss) {
	GError* error = nullptr;

	const SecretSchema schema = {
		"rog_control_center_password",	// name
		SECRET_SCHEMA_NONE,				// flags
		{								// attributes
		 {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
		 {NULL, (SecretSchemaAttributeType)0}},
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0  // reserved1..reserved7
	};

	secret_password_store_sync(&schema, SECRET_COLLECTION_DEFAULT, "Password for RogPerfTuner", pss.c_str(), nullptr, &error, "key", "default",
							   nullptr);

	if (error) {
		std::string msg = "Error saving password: " + std::string(error->message);
		g_error_free(error);
		throw new std::runtime_error(msg);
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