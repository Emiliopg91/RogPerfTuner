import { createContext, useContext } from "react";

export type Translate = (key: string, replacements?: Record<string, string>) => string;

export const I18nContext = createContext<Translate>((key) => key);

/// Translation function, backed by the backend's `resources/translations.yaml`
export function useTranslate(): Translate {
    return useContext(I18nContext);
}

export function buildTranslator(translations: Record<string, string>): Translate {
    return (key, replacements) => {
        let text = translations[key] ?? key;
        for (const [name, value] of Object.entries(replacements ?? {})) {
            text = text.replaceAll(`{${name}}`, value);
        }
        return text;
    };
}
