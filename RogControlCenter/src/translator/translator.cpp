
#include "../../include/translator/translator.hpp"

#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

Translator::Translator() {
	logger.debug("User language: {}", currentLang.toString());
}

std::string Translator::translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement) {
	std::optional<std::string_view> translation = std::nullopt;

	for (auto entry : initialTranslations) {
		if (entry.key == msg) {
			translation = entry.getTranslation(currentLang);
			break;
		}
	}

	if (translation.has_value()) {
		auto result = std::string(translation.value());

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
	} else {
		logger.warn("Missing translation for '{}'", msg);
		return msg;
	}
}