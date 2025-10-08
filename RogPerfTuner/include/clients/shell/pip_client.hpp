#pragma once

#include "../../models/others/singleton.hpp"
#include "abstract/abstract_cmd_client.hpp"

class PipClient : AbstractCmdClient, public Singleton<PipClient> {
  private:
	PipClient() : AbstractCmdClient("pip", "PipClient") {
	}
	friend class Singleton<PipClient>;

  public:
	/**
	 * @brief Installs a Python package using pip.
	 *
	 * This function attempts to install the specified Python package using the pip package manager.
	 *
	 * @param package The name of the Python package to install.
	 * @param breakSystemPackages If true, allows installation even if it may break system packages (default: false).
	 */
	void installPackage(std::string package, bool breakSystemPackages = false);
};