import os
import shutil
import subprocess
from threading import Lock, Thread
import time
from framework.logger import Logger
from rcc.utils.beans import CRYPTOGRAPHY
from rcc.utils.configuration import CONFIGURATION
from rcc.utils.constants import ASSET_SCRIPTS_FOLDER, REST_SERVER_PORT, STEAM_BIN_PATH, USER_BIN_FOLDER, USER_LOG_FOLDER


class Shell:
    """Class for run shell commands"""

    RC_MARKER = "__RC__"
    END_MARKER = "__END__"
    std_session = None
    sudo_session = None
    STD_LOCK = Lock()
    SUDO_LOCK = Lock()

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

    def __read_until_end(self, stream, end_marker, output_list, rc_holder=None):
        for line in stream:
            line = line.rstrip()
            if line.startswith(self.RC_MARKER):
                if rc_holder is not None:
                    try:
                        rc_holder[0] = int(line[len(self.RC_MARKER) :])
                    except ValueError:
                        rc_holder[0] = -1
                continue
            if line.strip() == end_marker:
                break
            output_list.append(line)

    def __init_std_session(self):
        self.std_session = subprocess.Popen(  # pylint:disable=consider-using-with
            ["bash"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            env=os.environ.copy(),
        )

    def __init_sudo_session(self):
        if self.__pwd is None:
            self.__pwd = CRYPTOGRAPHY.decrypt_string(CONFIGURATION.settings.password) + "\n"

        self.sudo_session = subprocess.Popen(  # pylint:disable=consider-using-with
            ["sudo", "-S", "bash"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            env=os.environ.copy(),
        )

        self.sudo_session.stdin.write(self.__pwd)
        self.sudo_session.stdin.write(f"echo {self.END_MARKER}\n")
        self.sudo_session.stdin.write(f"echo {self.RC_MARKER}$rc 1>&2\n")
        self.sudo_session.stdin.write(f"echo {self.END_MARKER} 1>&2\n")
        self.sudo_session.stdin.flush()

        stdout_lines = []
        stderr_lines = []
        t_out = Thread(target=self.__read_until_end, args=(self.sudo_session.stdout, self.END_MARKER, stdout_lines))
        t_err = Thread(target=self.__read_until_end, args=(self.sudo_session.stderr, self.END_MARKER, stderr_lines))

        t_out.start()
        t_err.start()
        t_out.join()
        t_err.join()

    def __run(self, session, lock, command, check=True):
        rc_holder = [None]
        stdout_lines = []
        stderr_lines = []

        with lock:

            self.__logger.debug(f"Running {"privileged "if session == self.sudo_session else ""}command '{command}'")
            t0 = time.time()
            session.stdin.write(f"{command}\n")
            session.stdin.write("rc=$?\n")
            session.stdin.write(f"echo {self.END_MARKER}\n")
            session.stdin.write(f"echo {self.RC_MARKER}$rc 1>&2\n")
            session.stdin.write(f"echo {self.END_MARKER} 1>&2\n")
            session.stdin.flush()

            t_out = Thread(target=self.__read_until_end, args=(session.stdout, self.END_MARKER, stdout_lines))
            t_err = Thread(
                target=self.__read_until_end, args=(session.stderr, self.END_MARKER, stderr_lines, rc_holder)
            )

            t_out.start()
            t_err.start()
            t_out.join()
            t_err.join()

        stdout = os.linesep.join(stdout_lines)
        stderr = os.linesep.join(stderr_lines)
        if rc_holder[0] is not None:
            excode = int(rc_holder[0])
        else:
            excode = 127
        self.__logger.debug(f"Command finished with result {excode} after {(time.time()-t0):.3f}")

        if check and excode != 0:
            raise subprocess.CalledProcessError(excode, command, stdout, stderr)

        return excode, stdout, stderr

    def run_command(self, command, check=True):
        """Run command in shared session"""
        if self.std_session is None:
            self.__init_std_session()
        return self.__run(self.std_session, self.STD_LOCK, command, check)

    def run_sudo_command(self, command, check=True):
        """Run sudo command in shared session"""
        if self.sudo_session is None:
            self.__init_sudo_session()
        return self.__run(self.sudo_session, self.SUDO_LOCK, command, check)

    def run_command_in_new_session(self, command, sudo=False, check=True, output=False):
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
