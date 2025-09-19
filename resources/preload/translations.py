import json
import os
import sys
import re

input_file = os.path.abspath(
    os.path.dirname(__file__) + "/../../resources/translations.json"
)
translator_file = os.path.abspath(
    os.path.dirname(__file__) + "/../../RogControlCenter/src/translator/translator.cpp"
)

print(f"Preloading translations from {input_file}")

if os.path.exists(translator_file):
    mtime1 = os.path.getmtime(input_file)
    mtime2 = os.path.getmtime(translator_file)
    if mtime1 <= mtime2:
        print("    Preload up to date")
        sys.exit(0)

with open(input_file, "r", encoding="utf-8") as f:
    translations: dict[str, dict[str, str]] = json.load(f)


dec = f"std::vector<TranslationEntry> Translator::translations  = {{\n"
for entry in translations:
    dec = dec+ f'{{ "{entry}",'
    for lang in ["en", "es"]:
        tr = translations[entry].get(lang, "")
        dec =dec+ f'"{tr}",'
    dec =dec+ "},\n"
dec = dec+"};\n"

with open(translator_file, "r", encoding="utf-8") as f:
    contenido = f.read()

nuevo_contenido = re.sub(
    r"(// Begin translations)(.*?)(// End translations)",
    lambda m: f"{m.group(1)}\n    {dec}\n    {m.group(3)}",
    contenido,
    flags=re.DOTALL,
)

with open(translator_file, "w", encoding="utf-8") as f:
    f.write(nuevo_contenido)

print(f"    Updated translator.hpp and translations.cpp with {len(translations)} entries")
