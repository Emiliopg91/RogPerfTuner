#pragma once

#include <string>

#include "language.hpp"

struct TranslationEntry {
	std::string key;
	std::string en;
	std::string es;

	std::string getTranslation(const Language& lang);
};