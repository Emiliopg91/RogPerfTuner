
#include "framework/translator/translator.hpp"

Translator::Translator() : Loggable("Translator") {
}

void Translator::loadTranslations(const std::unordered_map<std::string, std::string>& translations) {
	this->translations = translations;
}

std::string Translator::translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement) {
	auto result = translations.find(msg)->second;
	for (const auto& [key, value] : replacement) {
		std::string placeholder = std::format("{{{}}}", key);
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
