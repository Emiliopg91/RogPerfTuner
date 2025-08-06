from abc import ABC

from framework.logger import Logger
from rcc.utils.shell import SHELL


class AbstractFileClient(ABC):
    """Abstract class for file client"""

    def __init__(self, path: str):
        self.__path = path
        self._logger = Logger(self.__class__.__name__)
        self._available = SHELL.run_command(f"ls {self.__path}", check=False) == 0

    @property
    def available(self):
        """Availability flag"""
        return self._available

    def read(self, head=0, tail=0):
        """Read content from file with head and tail optional"""
        cmd = f"cat {self.__path}"
        if head > 0:
            cmd += f" | head -n{head}"
        if tail > 0:
            cmd += f" | tail -n{tail}"

        return SHELL.run_command(cmd)[1]

    def write(self, content, sudo=False):
        """Write content to file"""
        if sudo:
            SHELL.run_sudo_command(f"echo '{content}' | tee {self.__path}")

        SHELL.run_command(f"echo '{content}' | tee {self.__path}")
