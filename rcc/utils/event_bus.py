from typing import Any, Callable

from rcc.utils.logger import Logger
from rcc.utils.singleton import singleton


@singleton
class EventBus:
    """Class for event bus"""

    def __init__(self):
        self.__logger = Logger()
        self._callbacks: dict[str, list[Callable[..., None]]] = {}

    def on(self, event: str, callback: Callable[..., None]) -> None:
        """Define listener for event"""
        try:
            self._callbacks[event].append(callback)
        except KeyError:
            self._callbacks[event] = []
            self._callbacks[event].append(callback)
        self.__logger.debug(f"Registered callback for {event}")

    def emit(self, event: str, *args: Any) -> None:
        """Emit event"""
        self.__logger.debug(f"Emitting event {event} with args ({str(tuple(args))[1:-2]})")
        try:
            for i in range(len(self._callbacks[event])):
                try:
                    self._callbacks[event][i](*args)
                except Exception as e:
                    self.__logger.error(f"Error on callback: {e}")
        except KeyError:
            """"""


event_bus = EventBus()
