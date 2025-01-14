from abc import ABC
import os
import json
from typing import Callable
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

from lib.utils.logger import Logger


class FileCreationHandler(FileSystemEventHandler):
    """Class for file watching"""

    def __init__(self, callback):
        self.callback = callback

    def on_created(self, event):
        # Detectar si el evento es un archivo
        if not event.is_directory:
            print(f"Archivo creado: {event.src_path}")
            self.callback(event.src_path)


class AbstractFileClient(ABC):
    """Abstract class for file clients"""

    def __init__(self, path: str):
        super().__init__()
        self._path = path
        self._logger = Logger()
        self.__callbacks: dict[str, list[Callable[..., None]]] = {}

        event_handler = FileCreationHandler(self._on_file_event)
        observer = Observer()
        observer.schedule(event_handler, self._path, recursive=False)

        print(f"Observando el directorio: {self._path}")
        observer.start()

    def _on_file_event(self, file_path: str):
        parts = os.path.basename(file_path).split(".")[0].split("-")
        with open(file_path, "r") as file:
            data = json.load(file)
        event = parts[0]

        self._logger.info(f"Emiting event {parts[0]} with data {data}")
        if event in self.__callbacks:
            for callback in self.__callbacks[event]:
                try:
                    callback(*data)
                except Exception:
                    pass

    def on(self, event: str, callback: Callable[..., None]):
        """Event callback register"""
        if event not in self.__callbacks:
            self.__callbacks[event] = []

        if callback not in self.__callbacks[event]:
            self.__callbacks[event].append(callback)
