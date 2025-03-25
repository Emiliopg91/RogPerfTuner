from framework.cryptography import Cryptography
from framework.event_bus import EventBus
from framework.translator import Translator
from rcc.utils import constants

CRYPTOGRAPHY = Cryptography(constants.APP_NAME)
EVENT_BUS = EventBus()
TRANSLATOR = Translator(constants.TRANSLATIONS_PATH)
