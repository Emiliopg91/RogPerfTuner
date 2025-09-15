#pragma once

#include "../logger/logger.hpp"
#include "../models/others/singleton.hpp"
#include "../models/settings/root_config.hpp"

class Configuration : public Singleton<Configuration> {
  public:
	void saveConfig();
	void loadConfig();

	RootConfig& getConfiguration();

	std::string getPassword();
	void setPassword(const std::string& pss);

  private:
	Configuration();
	std::optional<RootConfig> config = std::nullopt;
	Logger logger{"Configuration"};
	friend class Singleton<Configuration>;
};