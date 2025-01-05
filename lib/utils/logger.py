import inspect
import logging
import os
import shutil

from lib.utils.constants import log_file, log_folder, log_old_folder


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
            if not os.path.exists(log_old_folder):
                os.makedirs(log_old_folder, exist_ok=True)

            for lf in [file for file in os.listdir(log_folder) if file.endswith(".log")]:
                shutil.move(os.path.join(log_folder, lf), log_old_folder)

            old_files = [
                os.path.join(log_old_folder, archivo)
                for archivo in os.listdir(log_old_folder)
                if archivo.endswith(".log")
            ]
            old_files.sort(key=os.path.getmtime)
            to_rem = old_files[:-10] if len(old_files) > 10 else []

            for rf in to_rem:
                os.unlink(rf)

            formatter = logging.Formatter(
                "[%(asctime)s.%(msecs)03d][%(levelname)-8s][%(name)-20s] - %(message)s",
                datefmt="%Y-%m-%d %H:%M:%S",
            )

            Logger.file_handler = logging.FileHandler(log_file)
            Logger.file_handler.setFormatter(formatter)

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
        self.logger.info(Logger._tab_msg(message))

    def debug(self, message: str):
        """Write debug log"""
        self.logger.debug(Logger._tab_msg(message))

    def warning(self, message: str):
        """Write warning log"""
        self.logger.warning(Logger._tab_msg(message))

    def error(self, message: str):
        """Write error log"""
        self.logger.error(Logger._tab_msg(message))

    def critical(self, message: str):
        """Write critical log"""
        self.logger.critical(Logger._tab_msg(message))

    def add_tab(self):
        """Add tab to logger"""
        Logger.tabs += 1

    def rem_tab(self):
        """Remove tab from logger"""
        Logger.tabs -= 1

    @staticmethod
    def _tab_msg(msg: str):
        return ("  " * Logger.tabs) + msg
