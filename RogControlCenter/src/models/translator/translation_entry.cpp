#include "../../../include/models/translator/translation_entry.hpp"

std::string TranslationEntry::getTranslation(const Language& lang) {
	std::optional<std::string> tr = std::nullopt;

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