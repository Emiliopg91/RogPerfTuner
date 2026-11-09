//! Translations loaded from the shared `resources/translations.yaml` (key -> language -> text).
//! The language is picked at runtime from `LANG`.

use std::{collections::HashMap, env, sync::LazyLock};

const TRANSLATIONS_YAML: &str = include_str!("../../../resources/translations.yaml");

static LANGUAGE: LazyLock<String> = LazyLock::new(|| {
    ["LC_ALL", "LC_MESSAGES", "LANG"]
        .iter()
        .filter_map(|v| env::var(v).ok())
        .find(|v| !v.is_empty() && v != "C" && v != "POSIX")
        .map(|v| v.chars().take(2).collect::<String>().to_lowercase())
        .unwrap_or_else(|| "en".to_string())
});

static TRANSLATIONS: LazyLock<HashMap<String, String>> = LazyLock::new(|| {
    let all: HashMap<String, HashMap<String, String>> = noyalib::from_str(TRANSLATIONS_YAML).unwrap_or_default();
    all.into_iter()
        .filter_map(|(key, langs)| {
            langs
                .get(LANGUAGE.as_str())
                .or_else(|| langs.get("en"))
                .cloned()
                .map(|text| (key, text))
        })
        .collect()
});

pub fn language() -> &'static str {
    LANGUAGE.as_str()
}

/// Translates `key`, returning the key itself when there is no translation.
pub fn translate(key: &str) -> String {
    TRANSLATIONS.get(key).cloned().unwrap_or_else(|| key.to_string())
}

/// Translates `key` replacing every `{name}` placeholder.
pub fn translate_with(key: &str, replacements: &[(&str, &str)]) -> String {
    let mut text = translate(key);
    for (name, value) in replacements {
        text = text.replace(&format!("{{{name}}}"), value);
    }
    text
}

/// Every translation for the current language, sent to the frontend.
pub fn all() -> HashMap<String, String> {
    TRANSLATIONS.clone()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn translations_load() {
        assert!(!TRANSLATIONS.is_empty());
        assert_eq!(translate("missing.key.for.test"), "missing.key.for.test");
    }

    #[test]
    fn placeholders_get_replaced() {
        let text = translate_with("profile.applied", &[("profile", "XYZ")]);
        assert!(text.contains("XYZ"), "{text}");
    }
}
