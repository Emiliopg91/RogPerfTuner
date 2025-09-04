#pragma once

#include <any>

#include "RccCommons.hpp"

using TranslationMap = std::map<std::string, std::map<std::string, std::string>>;

class Translator {
  public:
	static Translator& getInstance() {
		static Translator instance;
		return instance;
	}

	std::string translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement = {});

  private:
	inline static std::string FALLBACK_LANG = "en";
	static TranslationMap translations;

	Translator();
	Logger logger{"Translator"};
	std::string currentLang = []() {
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
		return langStr;
	}();
};