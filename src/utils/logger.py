from .constants import log_file, log_folder

import logging
import os


class Logger:
    tabs = 0

    def __init__(self, class_name: str):
        if not os.path.exists(log_folder):
            os.makedirs(log_folder, exist_ok=True)

        self.class_name = class_name

        # Configurar el logger
        self.logger = logging.getLogger(self.class_name)
        self.logger.setLevel(logging.INFO)

        # Crear un formateador con el formato solicitado
        formatter = logging.Formatter(
            "[%(asctime)s.%(msecs)03d]][%(threadName)-10s][%(levelname)-8s][%(name)-20s] - %(message)s",
            datefmt="%Y-%m-%d %H:%M:%S",
        )

        # Crear un manejador de archivo
        file_handler = logging.FileHandler(log_file)
        file_handler.setFormatter(formatter)

        # Crear un manejador para la consola
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(formatter)

        # Agregar ambos manejadores al logger
        self.logger.addHandler(file_handler)
        self.logger.addHandler(console_handler)

    def info(self, message: str):
        self.logger.info(Logger._tag_msg(message))

    def debug(self, message: str):
        self.logger.debug(Logger._tag_msg(message))

    def warning(self, message: str):
        self.logger.warning(Logger._tag_msg(message))

    def error(self, message: str):
        self.logger.error(Logger._tag_msg(message))

    def critical(self, message: str):
        self.logger.critical(Logger._tag_msg(message))

    def addTab(self):
        Logger.tabs += 1

    def remTab(self):
        Logger.tabs -= 1

    @staticmethod
    def _tag_msg(msg: str):

        return ("  " * Logger.tabs) + msg
