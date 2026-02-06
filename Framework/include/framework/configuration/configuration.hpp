#pragma once

#include "framework/abstracts/loggable.hpp"
#include "framework/abstracts/singleton.hpp"
#include "framework/utils/file_utils.hpp"
#include "framework/utils/yaml_utils.hpp"

template <typename T>
class Configuration : public Singleton<Configuration<T>>, Loggable {
  public:
	/**
	 * @brief Saves the current configuration to persistent storage.
	 *
	 * This method serializes and writes the current configuration settings
	 * to a file or other persistent medium, ensuring that changes are retained
	 * across application restarts.
	 *
	 * @note Throws an exception if saving fails.
	 */
	void saveConfig() {
		if (!config.has_value()) {
			return;
		}

		logger->debug("Configuration saved");

		try {
			YamlUtils::writeYamlFile(*config, configFile);
		} catch (const std::exception& e) {
			logger->error("Error saving settings file: {}", e.what());
		}
	}
	/**
	 * @brief Loads the configuration settings from the appropriate source.
	 *
	 * This function initializes or updates the configuration by reading from
	 * a file, database, or other storage medium. It should be called before
	 * accessing any configuration-dependent functionality to ensure that the
	 * latest settings are applied.
	 *
	 * @note Throws an exception or handles errors internally if the configuration
	 *       cannot be loaded.
	 */
	void loadConfig() {
		if (FileUtils::exists(configFile)) {
			logger->debug("Loading settings from {}", configFile);
			try {
				config = YamlUtils::readYamlFile<T>(configFile);
				LoggerProvider::setConfigMap(config->logger);
			} catch (const std::exception& e) {
				logger->error("Error loading settings: {}", e.what());
			}
		} else {
			logger->debug("Settings file not found, creating new");
			FileUtils::createDirectory(configDir);

			T defaultCfg;
			config = defaultCfg;

			saveConfig();
		}
		logger->info("Configuration loaded");
	}

	/**
	 * @brief Retrieves the current root configuration.
	 *
	 * @return Reference to the RootConfig object containing the application's configuration settings.
	 */
	T& getConfiguration() {
		return config.value();
	}

  protected:
	Configuration(std::string dir, std::string file) : Loggable("Configuration"), configDir(dir), configFile(dir + "/" + file) {
		loadConfig();
	}

  private:
	friend class Singleton<Configuration<T>>;
	std::string configDir;
	std::string configFile;
	std::optional<T> config = std::nullopt;
};