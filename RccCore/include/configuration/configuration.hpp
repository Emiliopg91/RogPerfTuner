#pragma once

#include "../models/settings.hpp"
#include "RccCommons.hpp"

class Configuration
{
public:
    static Configuration &getInstance()
    {
        static Configuration instance;
        return instance;
    }

    void saveConfig();

    RootConfig &getConfiguration()
    {
        if (!config.has_value())
            config = RootConfig{};
        return config.value();
    }

    std::string getPassword();
    void setPassword(std::string pss);

private:
    Configuration()
    {
        loadConfig();
    }
    std::optional<RootConfig> config = std::nullopt;
    void loadConfig();
    Logger logger{"Configuration"};
};