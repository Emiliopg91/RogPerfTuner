
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
#include "../../include/utils/constants.hpp"
#include "../../include/translator/translator.hpp"

using json = nlohmann::json;

Translator::Translator()
{

    logger.debug("User language: " + currentLang);

    const std::string TRANSLATION_FILE = Constants::ASSETS_DIR + "/translations.json";
    logger.debug("Loading translations from " + TRANSLATION_FILE);
    try
    {
        std::ifstream f(TRANSLATION_FILE);
        if (!f)
        {
            logger.error("Couldn't load translations: {}", TRANSLATION_FILE);
            return;
        }

        nlohmann::json j;
        f >> j;

        std::map<std::string, std::map<std::string, std::string>> t2 = j.get<std::map<std::string, std::map<std::string, std::string>>>();
        for (auto &[key, entries] : j.items())
        {
            std::map<std::string, std::string> inner_map;
            for (auto &[lang, text] : entries.items())
            {
                if (lang == FALLBACK_LANG || lang == currentLang)
                    inner_map[lang] = text.get<std::string>();
            }
            translations[key] = std::move(inner_map);
        }
    }
    catch (const std::exception &e)
    {
        logger.error(fmt::format("Error loading translations: {}", e.what()));
    }
}

std::string Translator::translate(const std::string &msg, const std::unordered_map<std::string, std::any> &replacement)
{
    std::string result = msg;

    auto it = translations.find(msg);
    if (it != translations.end())
    {
        std::map<std::string, std::string> entry = it->second;
        auto it2 = entry.find(currentLang);
        if (it2 != entry.end())
        {
            result = it2->second;
        }
        else
        {
            result = entry.at(FALLBACK_LANG);
            logger.warn("Missing translation for '" + msg + "'");
        }

        for (const auto &[key, value] : replacement)
        {
            std::string placeholder = "{" + key + "}";
            std::string valStr;

            try
            {
                valStr = std::any_cast<std::string>(value);
            }
            catch (const std::bad_any_cast &)
            {
                try
                {
                    valStr = std::to_string(std::any_cast<int>(value));
                }
                catch (const std::bad_any_cast &)
                {
                    valStr = "<invalid>";
                }
            }

            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos)
            {
                result.replace(pos, placeholder.length(), valStr);
                pos += valStr.length();
            }
        }
    }
    else
    {
        logger.warn("Missing translation for '" + msg + "'");
    }

    return result;
}