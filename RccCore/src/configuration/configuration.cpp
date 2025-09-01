#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libsecret/secret.h>

#include "RccCommons.hpp"

#include "../../include/configuration/configuration.hpp"

void Configuration::loadConfig()
{
    if (FileUtils::exists(Constants::CONFIG_FILE))
    {
        logger.debug("Loading settings from '{}'", Constants::CONFIG_FILE);
        try
        {
            std::ifstream f(Constants::CONFIG_FILE);
            if (!f)
            {
                logger.error("Couldn't load settings: {}", Constants::CONFIG_FILE);
                return;
            }

            nlohmann::json j;
            f >> j;

            // Convierte JSON → RootConfig
            RootConfig cfg = j.get<RootConfig>();
            config = cfg;

            LoggerProvider::setConfigMap(cfg.logger);
        }
        catch (const std::exception &e)
        {
            logger.error(fmt::format("Error loading settings: {}", e.what()));
        }
    }
    else
    {
        logger.debug("Settings file not found, creating new");
        FileUtils::createDirectory(Constants::CONFIG_DIR);

        // Configuración por defecto
        RootConfig defaultCfg;
        config = defaultCfg;

        saveConfig();
    }
}

void Configuration::saveConfig()
{
    if (!config.has_value())
        return;

    try
    {
        std::ofstream f(Constants::CONFIG_FILE);
        if (!f)
        {
            logger.error("Couldn't save settings file '{}'", Constants::CONFIG_FILE);
            return;
        }

        nlohmann::json j = config.value();
        f << j.dump(2); // indentación de 2 espacios
    }
    catch (const std::exception &e)
    {
        logger.error("Error saving settings file: '{}'", e.what());
    }
}

std::string Configuration::getPassword()
{
    GError *error = nullptr;

    const SecretSchema schema = {
        "rog_control_center_password", // name
        SECRET_SCHEMA_NONE,            // flags
        {                              // attributes
         {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
         {NULL, (SecretSchemaAttributeType)0}},
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0 // reserved1..reserved7
    };

    gchar *password = secret_password_lookup_sync(
        &schema,
        nullptr,
        &error,
        "key", "default",
        nullptr);

    if (error)
    {
        std::string msg = "Error fetching password: " + std::string(error->message);
        g_error_free(error);
        throw new std::runtime_error(msg);
    }

    std::string result = password ? password : "";
    g_free(password);
    return result;
}

void Configuration::setPassword(std::string pss)
{

    GError *error = nullptr;

    const SecretSchema schema = {
        "rog_control_center_password", // name
        SECRET_SCHEMA_NONE,            // flags
        {                              // attributes
         {"key", SECRET_SCHEMA_ATTRIBUTE_STRING},
         {NULL, (SecretSchemaAttributeType)0}},
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0 // reserved1..reserved7
    };

    secret_password_store_sync(
        &schema,
        SECRET_COLLECTION_DEFAULT,
        "Password for RogControlCenter",
        pss.c_str(),
        nullptr,
        &error,
        "key", "default",
        nullptr);

    if (error)
    {
        std::string msg = "Error saving password: " + std::string(error->message);
        g_error_free(error);
        throw new std::runtime_error(msg);
    }
}