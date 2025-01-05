from typing import Any, Callable

from lib.utils.singleton import singleton


@singleton
class EventBus:
    """Class for event bus"""

    def __init__(self):
        self._callbacks: dict[str, list[Callable[[Any], None]]] = {}

    def on(self, event: str, callback: Callable[[Any], None]) -> None:
        """Define listener for event"""
        try:
            self._callbacks[event].append(callback)
        except KeyError:
            self._callbacks[event] = []
            self._callbacks[event].append(callback)

    def emit(self, event: str, value: any = None) -> None:
        """Emit event"""
        try:
            for i in range(len(self._callbacks[event])):
                try:
                    self._callbacks[event][i](value)
                except Exception as e:
                    print(e)
        except KeyError:
            """"""


event_bus = EventBus()
