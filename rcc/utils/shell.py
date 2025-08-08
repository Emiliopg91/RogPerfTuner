import os
import shutil
import subprocess
import time
from framework.logger import Logger
from rcc.utils.beans import CRYPTOGRAPHY
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.constants import ASSET_SCRIPTS_FOLDER, REST_SERVER_PORT, STEAM_BIN_PATH, USER_BIN_FOLDER, USER_LOG_FOLDER


class Shell:
    """Class for run shell commands"""

    def __init__(self):
        self.__logger = Logger()
        self.__pwd = None
        os.makedirs(USER_BIN_FOLDER, exist_ok=True)
        self.__copy_scripts(
            {
                "rest_port": str(REST_SERVER_PORT),
                "logger_path": USER_LOG_FOLDER,
                "steam_path": STEAM_BIN_PATH,
            }
        )

    def __run(self, command, check, sudo):
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
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            shell=True,
        )
        self.__logger.debug(f"Command finished with result {proccess.returncode} after {(time.time()-t0):.3f}")

        return (proccess.returncode, proccess.stdout, proccess.stderr)

    def run_command(self, command, check=True):
        """Run command"""
        return self.__run(command, check, False)

    def run_sudo_command(self, command, check=True):
        """Run sudo command"""
        return self.__run(command, check, True)

    def copy_scripts(self, folder, replacements=None):
        """Copy scripts"""
        if os.path.isdir(os.path.join(USER_BIN_FOLDER, folder)):
            shutil.rmtree(os.path.join(USER_BIN_FOLDER, folder))

        os.makedirs(os.path.join(USER_BIN_FOLDER, folder), exist_ok=True)
        for file in os.listdir(os.path.join(ASSET_SCRIPTS_FOLDER, folder)):
            src_path = os.path.join(ASSET_SCRIPTS_FOLDER, folder, file)
            dst_path = os.path.join(USER_BIN_FOLDER, folder, file)

            if os.path.isfile(dst_path):
                os.unlink(dst_path)

            with open(src_path, "r") as f:
                content = f.read()

            if replacements is not None:
                for key in replacements:
                    content = content.replace("{{" + key + "}}", str(replacements[key]))

            with open(dst_path, "w") as f:
                f.write(content)

            os.chmod(dst_path, 0o755)

    def __copy_scripts(self, replacements):
        """Copy scripts from assets"""
        for folder in os.listdir(ASSET_SCRIPTS_FOLDER):
            if folder != "application":
                self.copy_scripts(folder, replacements)


SHELL = Shell()
