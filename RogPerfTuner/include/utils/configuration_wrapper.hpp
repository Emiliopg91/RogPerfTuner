#pragma once

#include "abstracts/singleton.hpp"
#include "configuration/configuration.hpp"
#include "models/settings/root_config.hpp"

class ConfigurationWrapper : public Singleton<ConfigurationWrapper>, public Configuration<RootConfig> {
  public:
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
	ConfigurationWrapper();
	friend class Singleton<ConfigurationWrapper>;

	std::optional<RootConfig> config = std::nullopt;
};