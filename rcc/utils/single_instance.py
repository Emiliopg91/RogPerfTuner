import os
import sys
import time
import psutil

from rcc.utils.constants import LOCK_FILE
from framework.singleton import singleton


@singleton
class SingleInstance:
    """Application single instance handler"""

    def acquire(self, retry=False) -> None:
        """Try to acquire lock file"""
        if os.path.exists(LOCK_FILE):
            with open(LOCK_FILE, "r") as f:
                pid = int(f.read().strip())

            if psutil.pid_exists(pid):
                if not retry:
                    time.sleep(1)
                    self.acquire(True)
                else:
                    print(f"Application already running with pid {pid}")
                    sys.exit(1)
            else:
                os.remove(LOCK_FILE)

        with open(LOCK_FILE, "w") as f:
            f.write(str(os.getpid()))


single_instance = SingleInstance()
