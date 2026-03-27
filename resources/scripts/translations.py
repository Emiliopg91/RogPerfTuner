import os
import yaml

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
TRANSLATIONS_FILE = os.path.join(BASE_DIR, "resources", "translations.yaml")
TRANSLATIONS_HPP = os.path.join(
    BASE_DIR, "RogPerfTuner", "include", "utils", "translations.hpp"
)

DEFAULT_LANG = "en"


def detect_language():
    lang = os.getenv("LC_MESSAGES")
    if not lang:
        lang = os.getenv("LANG")
    if not lang:
        return DEFAULT_LANG

    lang_str = lang

    if "_" in lang_str:
        lang_str = lang_str.split("_")[0]

    return lang_str


if __name__ == "__main__":
    proceed = not os.path.exists(TRANSLATIONS_HPP) or os.path.getmtime(
        TRANSLATIONS_FILE
    ) > os.path.getmtime(TRANSLATIONS_HPP)

    if proceed:
        lang = detect_language()
        print(f"Generating translation file for {lang.upper()}...")

        with open(TRANSLATIONS_FILE, "r", encoding="utf-8") as f:
            translations: dict[str, str] = yaml.safe_load(f)
        final_translations = {}

        for key, entries in translations.items():
            cl = lang
            if cl not in entries:
                cl = DEFAULT_LANG
            final_translations[key] = entries[cl]

        content = []
        content.append("#include <string>")
        content.append("#include <unordered_map>")
        content.append("")
        content.append(
            "inline std::unordered_map<std::string, std::string> getTranslations() {"
        )
        content.append("    return {")
        for key, value in final_translations.items():
            content.append(f'    {{"{key}", "{value}"}},')
        content.append("    };")
        content.append("}")

        with open(TRANSLATIONS_HPP, "w", encoding="utf-8") as f:
            f.write("\n".join(content))
