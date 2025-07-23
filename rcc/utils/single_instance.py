import os
import psutil

from rcc.utils.constants import LOCK_FILE
from framework.singleton import singleton


@singleton
class SingleInstance:
    """Application single instance handler"""

    def acquire(self, name) -> None:
        """Try to acquire lock file"""
        if os.path.exists(LOCK_FILE):
            with open(LOCK_FILE, "r") as f:
                pid = int(f.read().strip())

            if psutil.pid_exists(pid) and psutil.Process(pid).name().startswith(name):
                print(f"Application already running with pid {pid}, killing...")
                psutil.Process(pid).kill()
            else:
                os.remove(LOCK_FILE)

        with open(LOCK_FILE, "w") as f:
            f.write(str(os.getpid()))


SINGLE_INSTANCE = SingleInstance()
