from enum import Enum
from functools import wraps
import inspect
import logging
import math
import os
import shutil
import time

from rcc.utils.constants import dev_mode, log_file, log_folder, log_old_folder
from rcc.utils.configuration import configuration


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
                "[%(asctime)s.%(msecs)03d][%(levelname)-8s][%(name)-15s]%(message)s",
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
        self._class_name = class_name

        self._logger = logging.getLogger(self._class_name)
        self._logger.addHandler(Logger.file_handler)
        self._logger.addHandler(Logger.console_handler)

        level = configuration.logger.get(self._class_name, "INFO").upper()
        if level == "CRITICAL":
            self._logger.setLevel(logging.CRITICAL)
        elif level == "ERROR":
            self._logger.setLevel(logging.ERROR)
        elif level == "WARNING":
            self._logger.setLevel(logging.WARNING)
        elif level == "DEBUG":
            self._logger.setLevel(logging.DEBUG)
        else:
            self._logger.setLevel(logging.INFO)

    def _is_level_enabled(self, level: int) -> bool:
        return level >= self._logger.level

    def debug(self, message: str):
        """Write debug log"""
        self._logger.debug(Logger._format_msg(message))

    def is_debug_enabled(self) -> bool:
        """Check logging level"""
        return self._is_level_enabled(logging.DEBUG)

    def info(self, message: str):
        """Write info log"""
        self._logger.info(Logger._format_msg(message))

    def is_info_enabled(self) -> bool:
        """Check logging level"""
        return self._is_level_enabled(logging.INFO)

    def warning(self, message: str):
        """Write warning log"""
        self._logger.warning(Logger._format_msg(message))

    def is_warning_enabled(self) -> bool:
        """Check logging level"""
        return self._is_level_enabled(logging.WARNING)

    def error(self, message: str):
        """Write error log"""
        self._logger.error(Logger._format_msg(message))

    def is_error_enabled(self) -> bool:
        """Check logging level"""
        return self._is_level_enabled(logging.ERROR)

    def critical(self, message: str):
        """Write critical log"""
        self._logger.critical(Logger._format_msg(message))

    def is_critical_enabled(self) -> bool:
        """Check logging level"""
        return self._is_level_enabled(logging.CRITICAL)

    def add_tab(self):
        """Add tab to logger"""
        Logger.tabs += 1

    def rem_tab(self):
        """Remove tab from logger"""
        Logger.tabs -= 1

    @staticmethod
    def _format_msg(msg: str):
        frame = inspect.currentframe().f_back.f_back
        if dev_mode:
            method_name = (frame.f_code.co_name if frame else "UnknownMethod").ljust(20, " ")[:20]
            return f"[{method_name}] - {"  " * Logger.tabs}{msg}"
        return f"- {"  " * Logger.tabs}{msg}"


def logged_method(method):
    """Decorator for logging method invokation"""
    logger = Logger("logged_method")

    @wraps(method)
    def wrapped_method(*args, **kwargs):
        if logger.is_debug_enabled():
            fargs = args
            if len(args) > 0 and hasattr(args[0], "__class__"):
                fargs = args[1:]
            pargs_repr = ", ".join(a.name if isinstance(a, Enum) else repr(a) for a in fargs)
            kwargs_repr = ", ".join(f"{k}={v!r}" for k, v in kwargs.items()) if kwargs is not None else ""
            args_repr = (
                pargs_repr
                + (", " if args is not None and len(args) > 0 and kwargs is not None and len(kwargs) > 0 else "")
                + kwargs_repr
            )
            logger.info(f"Invoking method {method.__name__}({args_repr})")
            logger.add_tab()
            t0 = time.time()
        result = method(*args, **kwargs)
        if logger.is_debug_enabled():
            t1 = time.time()
            logger.rem_tab()
            logger.info(f"Invocation finished after {math.floor((t1-t0)*1000)} ms with result {result}")
        return result

    return wrapped_method
