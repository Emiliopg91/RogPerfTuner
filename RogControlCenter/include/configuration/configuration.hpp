#pragma once

#include "../logger/logger.hpp"
#include "../models/settings/root_config.hpp"

class Configuration {
  public:
	static Configuration& getInstance() {
		static Configuration instance;
		return instance;
	}

	void saveConfig();
	void loadConfig();

	RootConfig& getConfiguration();

	std::string getPassword();
	void setPassword(const std::string& pss);

  private:
	Configuration();
	std::optional<RootConfig> config = std::nullopt;
	Logger logger{"Configuration"};
};