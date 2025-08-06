from abc import ABC
import shutil

from rcc.utils.shell import SHELL


class AbstractCmdClient(ABC):
    """Abstract class for commands"""

    def __init__(self, command: str):
        self.__command = command

        self._available = shutil.which(self.__command) is not None

    @property
    def available(self):
        """Availability flag"""
        return self._available

    def _run_command(self, args="", sudo=False):
        if sudo:
            return SHELL.run_sudo_command(f"{self.__command} {args}".strip(), True)

        return SHELL.run_command(f"{self.__command} {args}".strip(), True)
