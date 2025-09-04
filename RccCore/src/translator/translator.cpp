
#include "../../include/translator/translator.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "RccCommons.hpp"

using json = nlohmann::json;

Translator::Translator() {
	logger.debug("User language: " + currentLang);

	try {
		for (auto& [key, entries] : translations) {
			for (auto it = entries.begin(); it != entries.end();) {
				if (it->first != FALLBACK_LANG && it->first != currentLang) {
					it = entries.erase(it);
				} else {
					++it;
				}
			}
		}
	} catch (const std::exception& e) {
		logger.error(fmt::format("Error loading translations: {}", e.what()));
	}
}

std::string Translator::translate(const std::string& msg,
								  const std::unordered_map<std::string, std::any>& replacement) {
	std::string result = msg;

	auto it = translations.find(msg);
	if (it != translations.end()) {
		std::map<std::string, std::string> entry = it->second;
		auto it2								 = entry.find(currentLang);
		if (it2 != entry.end()) {
			result = it2->second;
		} else {
			result = entry.at(FALLBACK_LANG);
			logger.warn("Missing translation for '" + msg + "'");
		}

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