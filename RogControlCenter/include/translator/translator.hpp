#pragma once

#include <any>
#include <string>
#include <unordered_map>

#include "../models/others/loggable.hpp"
#include "../models/others/singleton.hpp"
#include "../models/translator/translation_entry.hpp"

class Translator : public Singleton<Translator>, Loggable {
  public:
	std::string translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement = {});

  private:
	inline static Language FALLBACK_LANG = Language::Enum::EN;
	static std::vector<TranslationEntry> translations;

	friend class Singleton<Translator>;
	Translator();
	Language currentLang = Language::Enum::EN;
};