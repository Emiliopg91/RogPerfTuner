#pragma once

#include <unordered_map>

#include "abstracts/singleton.hpp"
#include "clients/shell/abstract/abstract_cmd_client.hpp"

class ScxCtlClient : public AbstractCmdClient, public Singleton<ScxCtlClient> {
  private:
	friend class Singleton<ScxCtlClient>;

	std::optional<std::string> current;
	std::unordered_map<std::string, std::string> available_sched;

	ScxCtlClient();

  public:
	/**
	 * @brief Retrieves a list of available items.
	 *
	 * This function queries and returns a vector containing the names or identifiers
	 * of all currently available items relevant to the client.
	 *
	 * @return std::vector<std::string> A vector of strings representing the available items.
	 */
	std::vector<std::string> getAvailable();
	/**
	 * @brief Retrieves the current value or state as a string, if available.
	 *
	 * @return An optional string containing the current value if present;
	 *         std::nullopt otherwise.
	 */
	std::optional<std::string> getCurrent();
	/**
	 * @brief Starts a process or service with the specified name.
	 *
	 * @param name The name of the process or service to start.
	 */
	void start(std::string name);
	/** @brief Stops the current operation or process managed by the client.
	 *
	 * This function is typically used to halt any ongoing activities or services
	 * initiated by the client. The specific behavior depends on the implementation.
	 */
	void stop();
};