#pragma once

#include <optional>
#include <string_view>

#include "language.hpp"

struct TranslationEntry {
	std::string_view key;
	std::string_view en;
	std::string_view es;

	std::string_view getTranslation(const Language& lang) {
		std::optional<std::string_view> tr = std::nullopt;

		if (lang == Language::Enum::EN) {
			if (!es.empty()) {
				tr = en;
			}
		} else if (lang == Language::Enum::ES) {
			if (!es.empty()) {
				tr = es;
			}
		}

		if (tr.value().empty()) {
			tr = key;
		}

		return tr.value();
	}
};