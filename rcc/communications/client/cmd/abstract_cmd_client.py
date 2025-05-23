from abc import ABC

from rcc.utils.shell import SHELL


class AbstractCmdClient(ABC):
    """Abstract class for commands"""

    def __init__(self, command: str):
        self.__command = command

        self._available = SHELL.run_command(f"which {self.__command}", check=False) == 0

    @property
    def available(self):
        """Availability flag"""
        return self._available

    def _run_command(self, args: str, sudo=False):
        return SHELL.run_command(command=f"{self.__command} {args}", output=True, sudo=sudo, check=True)
