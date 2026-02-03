#pragma once

#include <any>
#include <string>
#include <unordered_map>

#include "abstracts/loggable.hpp"
#include "abstracts/singleton.hpp"
#include "models/language.hpp"

class Translator : public Singleton<Translator>, Loggable {
  public:
	/**
	 * @brief Translates a message key to the current language, with optional replacements.
	 *
	 * Looks up the translation for the given message key and replaces placeholders
	 * with the provided values from the replacement map.
	 *
	 * @param msg The message key to translate.
	 * @param replacement A map of placeholder names to values for substitution (optional).
	 * @return The translated string with replacements applied.
	 */
	std::string translate(const std::string& msg, const std::unordered_map<std::string, std::any>& replacement = {});

  private:
	inline static Language FALLBACK_LANG = Language::EN;

	friend class Singleton<Translator>;
	Translator(const std::string& translation_file = "");
	Language currentLang = Language::EN;
	std::unordered_map<std::string, std::string> translations;
};