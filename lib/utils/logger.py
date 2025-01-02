import inspect
import logging
import os

from lib.utils.constants import log_file, log_folder


class Logger:
    """Application logger"""

    tabs = 0
    initialized = False
    file_handler = None
    console_handler = None

    @staticmethod
    def _initialize():
        if not Logger.initialized:
            if not os.path.exists(log_folder):
                os.makedirs(log_folder, exist_ok=True)

            # Crear un formateador con el formato solicitado
            formatter = logging.Formatter(
                "[%(asctime)s.%(msecs)03d]][%(threadName)-15s][%(levelname)-8s][%(name)-20s] - %(message)s",
                datefmt="%Y-%m-%d %H:%M:%S",
            )

            # Crear un manejador de archivo
            Logger.file_handler = logging.FileHandler(log_file)
            Logger.file_handler.setFormatter(formatter)

            # Crear un manejador para la consola
            Logger.console_handler = logging.StreamHandler()
            Logger.console_handler.setFormatter(formatter)

            Logger.initialized = True

    def __init__(self, class_name: str = None):
        Logger._initialize()

        if class_name is None:
            frame = inspect.currentframe()
            while frame:
                cls = frame.f_locals.get("self", None)
                if cls and cls.__class__.__name__ != self.__class__.__name__:
                    class_name = cls.__class__.__name__
                    break
                frame = frame.f_back
            else:
                class_name = "Unknown"
        self.class_name = class_name

        self.logger = logging.getLogger(self.class_name)
        self.logger.setLevel(logging.INFO)
        self.logger.addHandler(Logger.file_handler)
        self.logger.addHandler(Logger.console_handler)

    def info(self, message: str):
        """Write info log"""
        self.logger.info(Logger._tag_msg(message))

    def debug(self, message: str):
        """Write debug log"""
        self.logger.debug(Logger._tag_msg(message))

    def warning(self, message: str):
        """Write warning log"""
        self.logger.warning(Logger._tag_msg(message))

    def error(self, message: str):
        """Write error log"""
        self.logger.error(Logger._tag_msg(message))

    def critical(self, message: str):
        """Write critical log"""
        self.logger.critical(Logger._tag_msg(message))

    def add_tab(self):
        """Add tab to logger"""
        Logger.tabs += 1

    def rem_tab(self):
        """Remove tab from logger"""
        Logger.tabs -= 1

    @staticmethod
    def _tag_msg(msg: str):
        return ("  " * Logger.tabs) + msg
