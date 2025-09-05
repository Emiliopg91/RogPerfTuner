import json
import os


input_file = os.path.abspath(
    os.path.dirname(__file__) + "/../../resources/translations.json"
)
output_file = os.path.abspath(
    os.path.dirname(__file__) + "/../../RccCore/include/translator/translations.hpp"
)

print(f"Preloading translations from {input_file}")

with open(input_file, "r") as f:
    translations: dict[str, dict[str, str]] = json.load(f)

with open(output_file, "w") as out:
    out.write("#include <map>\n")
    out.write("#include <string>\n\n")
    out.write('#include "translator.hpp"\n\n')

    out.write(
        "std::map<std::string, std::map<std::string, std::string>> initialTranslations = {\n"
    )
    for literal in translations:
        {out.write(f'{{"{literal}", {{\n')}
        for lang in translations[literal]:
            out.write(f'{{"{lang}", "{translations[literal][lang]}"}},\n')
        {out.write("}},\n")}
    out.write("};")


print(f"Preloaded {len(translations)} translations in {output_file}")
