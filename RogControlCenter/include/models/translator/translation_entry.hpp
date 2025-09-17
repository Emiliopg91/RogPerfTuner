#pragma once

#include <string_view>

#include "language.hpp"

struct TranslationEntry {
	std::string_view key;
	std::string_view en;
	std::string_view es;

	std::string_view getTranslation(const Language& lang);
};