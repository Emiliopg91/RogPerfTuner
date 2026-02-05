#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_cmd_client.hpp"

class FlatpakClient : public AbstractCmdClient, public Singleton<FlatpakClient> {
  private:
	FlatpakClient() : AbstractCmdClient("flatpak", "FlatpakClient") {
	}
	friend class Singleton<FlatpakClient>;

  public:
	/**
	 * @brief Checks if a Flatpak package is installed.
	 *
	 * This function determines whether a Flatpak package with the specified name is installed
	 * on the system. The search can be limited to the user installation or include the system-wide
	 * installation based on the value of the `userland` parameter.
	 *
	 * @param name The name or identifier of the Flatpak package to check.
	 * @param userland If true, checks only the user installation; if false, checks the system installation.
	 * @return true if the package is installed in the specified scope, false otherwise.
	 */
	bool checkInstalled(const std::string& name, bool userland);
	/**
	 * @brief Installs a Flatpak package by name.
	 *
	 * @param name The name or identifier of the Flatpak package to install.
	 * @param userland If true, installs the package for the current user only; if false, installs system-wide.
	 * @return true if the installation was successful, false otherwise.
	 */
	bool install(const std::string& name, bool userland);
	/**
	 * @brief Overrides the specified Flatpak application or runtime.
	 *
	 * This function attempts to override the configuration or behavior of a Flatpak
	 * package identified by its name. The override can be applied either at the
	 * user level or system-wide, depending on the value of the `userland` parameter.
	 *
	 * @param name The name of the Flatpak application or runtime to override.
	 * @param userland If true, the override is applied for the current user only; if false, it is applied system-wide.
	 * @return true if the override was successful, false otherwise.
	 */
	bool override(const std::string& name, bool userland);
};