from abc import ABC
import os
import shutil

from framework.logger import Logger
from rcc.utils.shell import SHELL


class AbstractCmdClient(ABC):
    """Abstract class for commands"""

    def __init__(self, command: str):
        self._logger = Logger()
        self.__command = shutil.which(command)
        if self.__command is None:
            self._logger.error(f"Command {command} not found in path {os.environ["PATH"]}")

    @property
    def available(self):
        """Availability flag"""
        return self.__command is not None

    def _run_command(self, args="", sudo=False):
        if sudo:
            return SHELL.run_sudo_command(f"{self.__command} {args}".strip(), True)

        return SHELL.run_command(f"{self.__command} {args}".strip(), True)
