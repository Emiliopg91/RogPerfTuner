import json
import locale

from rcc.utils.logger import Logger
from rcc.utils.constants import translations_path


class Translator:
    """Class for locale translation"""

    _translation_file = translations_path
    _language = locale.getdefaultlocale()[0].split("_")[0]  # pylint: disable=W4902

    def __init__(self):
        self._logger = Logger()
        self._logger.debug(f"Initializing translator for {Translator._language}")

        data = {}
        with open(Translator._translation_file, "r") as file:
            data = json.load(file)

        self._translations = {}
        for key in data.keys():
            try:
                self._translations[key] = data[key][Translator._language]
            except KeyError:
                self._translations[key] = data[key]["en"]

    def translate(self, msg: str, replacement: dict[str, any] = None) -> str:
        """Translate literal with optional replacements"""
        try:
            result: str = self._translations[msg]
            if replacement is not None:
                for key in replacement.keys():
                    result = result.replace("{" + key + "}", str(replacement[key]))

            return result
        except KeyError:
            self._logger.warning(f"Missing translation for '{msg}'")
            return msg


translator = Translator()
