
#include "../../include/translator/translator.hpp"

#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

Translator::Translator() {
	logger.debug("User language: {}", currentLang);

	try {
		std::optional<std::string> val = std::nullopt;
		for (auto entry : initialTranslations) {
			if (currentLang == "es") {
				val = entry.es;
			}

			translations[entry.key] = val.value_or(entry.en);
		}

		initialTranslations.clear();
		initialTranslations.shrink_to_fit();
	} catch (const std::exception& e) {
		logger.error("Error loading translations: {}", e.what());
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
		logger.warn("Missing translation for '{}'", msg);
	}

	return result;
}