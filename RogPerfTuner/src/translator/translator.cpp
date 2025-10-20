
#include "../../include/translator/translator.hpp"

#include <yaml-cpp/yaml.h>

#include <string>

#include "../../include/utils/constants.hpp"

Translator::Translator() : Loggable("Translator") {
	currentLang = [this]() -> Language {
		const char* lang = std::getenv("LC_MESSAGES");
		if (!lang || std::string(lang).empty()) {
			lang = std::getenv("LANG");
		}
		if (!lang || std::string(lang).empty()) {
			return FALLBACK_LANG;
		}
		std::string langStr(lang);

		auto pos = langStr.find('_');
		if (pos != std::string::npos) {
			langStr = langStr.substr(0, pos);
		}
		try {
			return Language::fromString(langStr);
		} catch (std::exception& e) {
			logger.warn("Unsupported language {}, fallback to {}", langStr, FALLBACK_LANG.toString());
			return FALLBACK_LANG;
		}
	}();

	YAML::Node root = YAML::LoadFile(Constants::TRANSLATIONS_FILE);

	for (auto it = root.begin(); it != root.end(); ++it) {
		const std::string key	= it->first.as<std::string>();
		const YAML::Node& value = it->second;

		std::string val = key;
		if (value[currentLang.toString()]) {
			val = value[currentLang.toString()].as<std::string>();
		} else if (value[FALLBACK_LANG.toString()]) {
			logger.warn("Missing specific translation for {}", key);
			val = value[FALLBACK_LANG.toString()].as<std::string>();
		} else {
			logger.warn("Missing specific and default translation for {}", key);
		}

		translations[key] = val;
	}

	logger.debug("User language: {}", currentLang.toString());
}

std::string Translator::translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement) {
	auto it = translations.find(msg);
	if (it == translations.end()) {
		logger.warn("Missing translation for {}", msg);
		return msg;
	}

	auto result = it->second;
	for (const auto& [key, value] : replacement) {
		std::string placeholder = "{" + key + "}";
		std::string valStr;

		try {
			valStr = std::any_cast<std::string>(value);
		} catch (const std::bad_any_cast&) {
			try {
				valStr = std::to_string(std::any_cast<int>(value));
			} catch (const std::bad_any_cast&) {
				valStr = "<invalid>";
			}
		}

		size_t pos = 0;
		while ((pos = result.find(placeholder, pos)) != std::string::npos) {
			result.replace(pos, placeholder.length(), valStr);
			pos += valStr.length();
		}
	}

	return result;
}
