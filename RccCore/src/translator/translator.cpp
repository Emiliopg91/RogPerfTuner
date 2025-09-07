
#include "../../include/translator/translator.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>

#include "../../include/translator/translations.hpp"
#include "RccCommons.hpp"

using json = nlohmann::json;

Translator::Translator() {
	logger.debug("User language: " + currentLang);

	try {
		for (auto& [key, entries] : initialTranslations) {
			std::string literal = "";
			auto it				= entries.find(currentLang);
			if (it != entries.end()) {
				literal = it->second;
			} else {
				logger.warn("Missing translation for '" + key + "'");
				auto it = entries.find(FALLBACK_LANG);
				if (it != entries.end()) {
					literal = it->second;
				}
			}
			translations[key] = literal;
		}

		initialTranslations.clear();
		std::map<std::string, std::map<std::string, std::string>>().swap(initialTranslations);
	} catch (const std::exception& e) {
		logger.error(fmt::format("Error loading translations: {}", e.what()));
	}
}

std::string Translator::translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement) {
	std::string result = msg;

	auto it = translations.find(msg);
	if (it != translations.end()) {
		result = it->second;

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

	} else {
		logger.warn("Missing translation for '" + msg + "'");
	}

	return result;
}