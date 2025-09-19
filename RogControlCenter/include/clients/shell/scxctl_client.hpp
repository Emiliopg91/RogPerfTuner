#pragma once

#include <unordered_map>

#include "../../models/others/singleton.hpp"
#include "abstract/abstract_cmd_client.hpp"

class ScxCtlClient : public AbstractCmdClient, public Singleton<ScxCtlClient> {
  private:
	friend class Singleton<ScxCtlClient>;

	std::optional<std::string> current;
	std::unordered_map<std::string, std::string> available_sched;

	ScxCtlClient();

  public:
	std::vector<std::string> getAvailable();
	std::optional<std::string> getCurrent();
	void start(std::string name);
	void stop();
};