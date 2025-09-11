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

	RootConfig& getConfiguration() {
		if (!config.has_value()) {
			config = RootConfig{};
		}
		return config.value();
	}

	std::string getPassword();
	void setPassword(const std::string& pss);

  private:
	Configuration() {
		loadConfig();
	}
	std::optional<RootConfig> config = std::nullopt;
	Logger logger{"Configuration"};
};