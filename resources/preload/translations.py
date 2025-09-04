import json
import sys

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, "r") as f:
    translations: dict[str, dict[str, str]] = json.load(f)

with open(output_file, "w") as out:
    out.write("#include <map>\n")
    out.write("#include <string>\n\n")
    out.write('#include "../../include/translator/translator.hpp"\n\n')

    out.write("TranslationMap Translator::translations = {\n")
    for literal in translations:
        {out.write(f'{{"{literal}", {{\n')}
        for lang in translations[literal]:
            out.write(f'{{"{lang}", "{translations[literal][lang]}"}},\n')
        {out.write("}},\n")}
    out.write("};")


print(f"Preloaded {len(translations)} translations in {output_file}")
