from concurrent.futures import ThreadPoolExecutor
from typing import Any, Callable

from rcc.utils.logger import Logger
from rcc.utils.singleton import singleton


@singleton
class EventBus:
    """Class for event bus"""

    def __init__(self):
        self.__logger = Logger()
        self._callbacks: dict[str, list[Callable[..., None]]] = {}
        self.__executor = ThreadPoolExecutor(max_workers=4, thread_name_prefix="EventBus")

    def on(self, event: str, callback: Callable[..., None]) -> None:
        """Define listener for event"""
        if event not in self._callbacks:
            self._callbacks[event] = []
        self._callbacks[event].append(callback)
        self.__logger.debug(f"Registered callback for {event}")

    def emit(self, event: str, *args: Any) -> None:
        """Emit event"""
        self.__logger.debug(f"Emitting event {event} with args ({str(tuple(args))[1:-2]})")
        count = 0
        if event in self._callbacks:
            for i in range(len(self._callbacks[event])):
                count += 1
                self.__executor.submit(self._safe_invoke, self._callbacks[event][i], *args)
        self.__logger.debug(f"Invoked {count} callbacks")

    def _safe_invoke(self, callback: Callable[..., None], *args: Any) -> None:
        try:
            callback(*args)
        except Exception as e:
            self.__logger.error(f"Error executing callback {callback}: {e}")


event_bus = EventBus()
