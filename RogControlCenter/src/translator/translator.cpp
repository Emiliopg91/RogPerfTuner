
#include "../../include/translator/translator.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "../../include/utils/constants.hpp"

using json = nlohmann::json;

Translator::Translator() : Loggable("Translator") {
	currentLang = []() -> Language {
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
		return Language::fromString(langStr);
	}();

	std::ifstream file(Constants::TRANSLATIONS_FILE);
	json j;
	file >> j;

	for (auto& [key, value] : j.items()) {
		std::string val = key;
		if (value.contains(currentLang.toString())) {
			val = value.at(currentLang.toString()).get<std::string>();
		} else if (value.contains(FALLBACK_LANG.toString())) {
			logger.warn("Missing specific translation for {}", key);
			val = value.at(FALLBACK_LANG.toString()).get<std::string>();
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