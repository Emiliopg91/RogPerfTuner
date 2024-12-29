from .logger import Logger
from .constants import translations_path

import json
import locale


class Translator:
    _translation_file = translations_path
    _language = locale.getdefaultlocale()[0].split("_")[0]

    def __init__(self):
        self.logger = Logger("Translator")
        self.logger.debug(f"Initializing translator for {Translator._language}")

        data = {}
        with open(Translator._translation_file, "r") as file:
            data = json.load(file)

        self._translations = {}
        for key in data.keys():
            try:
                self._translations[key] = data[key][Translator._language]
            except KeyError:
                self._translations[key] = data[key]["en"]

    def translate(self, msg: str, replacement: dict[str, any] = {}):
        try:
            result: str = self._translations[msg]
            for key in replacement.keys():
                result = result.replace("{" + key + "}", str(replacement[key]))

            return result
        except KeyError:
            self.logger.warning(f"Missing translation for '{msg}'")
            return msg


translator = Translator()
