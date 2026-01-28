#pragma once

#include "models/others/loggable.hpp"
#include "models/others/singleton.hpp"
#include "models/settings/root_config.hpp"

class Configuration : public Singleton<Configuration>, Loggable {
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
	void saveConfig();
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
	void loadConfig();

	/**
	 * @brief Retrieves the current root configuration.
	 *
	 * @return Reference to the RootConfig object containing the application's configuration settings.
	 */
	RootConfig& getConfiguration();

	/**
	 * @brief Retrieves the stored password.
	 *
	 * @return A std::string containing the password.
	 */
	std::string getPassword();
	/**
	 * @brief Sets the password to the specified value.
	 *
	 * @param pss The new password as a string.
	 */
	void setPassword(const std::string& pss);

  private:
	Configuration();
	friend class Singleton<Configuration>;

	std::optional<RootConfig> config = std::nullopt;
};