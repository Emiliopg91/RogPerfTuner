from framework.cryptography import Cryptography
from framework.event_bus import EventBus
from framework.translator import Translator
from rcc import __app_name__
from rcc.utils import constants

CRYPTOGRAPHY = Cryptography(__app_name__)
EVENT_BUS = EventBus()
TRANSLATOR = Translator(constants.TRANSLATIONS_PATH)
