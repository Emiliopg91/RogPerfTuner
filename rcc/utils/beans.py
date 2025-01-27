from framework.cryptography import Cryptography
from framework.event_bus import EventBus
from framework.translator import Translator
from rcc import __app_name__
from rcc.utils import constants

cryptography = Cryptography(__app_name__)
event_bus = EventBus()
translator = Translator(constants.translations_path)
