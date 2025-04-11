import subprocess
import time
from framework.logger import Logger
from rcc.utils.beans import CRYPTOGRAPHY
from rcc.utils.configuration import CONFIGURATION


class Shell:
    """Class for run shell commands"""

    def __init__(self):
        self.__logger = Logger()
        self.__pwd = None

    def run_command(self, command, sudo=False, check=True, output=False):
        """Run command on shell"""
        if sudo and self.__pwd is None:
            self.__pwd = CRYPTOGRAPHY.decrypt_string(CONFIGURATION.settings.password) + "\n"

        final_cmd = command
        input_txt = None
        if sudo:
            final_cmd = f'sudo -S bash -c "{final_cmd}"'
            input_txt = self.__pwd

        self.__logger.debug(f"Running command '{final_cmd}'")
        t0 = time.time()
        proccess = subprocess.run(
            final_cmd,
            input=input_txt,
            check=check,
            capture_output=False,
            stdout=subprocess.DEVNULL if not output else subprocess.PIPE,
            stderr=subprocess.DEVNULL if not output else subprocess.PIPE,
            text=True,
            shell=True,
        )
        self.__logger.debug(f"Command finished with result {proccess.returncode} after {(time.time()-t0):.3f}")

        return (proccess.returncode, proccess.stdout, proccess.stderr) if output else proccess.returncode


SHELL = Shell()
