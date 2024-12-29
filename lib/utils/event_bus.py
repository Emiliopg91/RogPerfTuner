from ..utils.singleton import singleton

from typing import Any, Callable

@singleton
class EventBus:
    def __init__(self):
        self.callbacks: dict[str, list[Callable[[Any], None]]] = {}

    def on(self, event: str, callback: Callable[[Any], None]):
        try:
            self.callbacks[event].append(callback)
        except KeyError:
            self.callbacks[event]=[]
            self.callbacks[event].append(callback)

    def emit(self, event: str, value: any):
        try:
            for i in range(len(self.callbacks[event])):
                try:
                    self.callbacks[event][i](value)
                except Exception as e:
                    print(e)
        except KeyError:
            """"""

event_bus = EventBus()