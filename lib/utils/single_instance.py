from .constants import lock_file
from .singleton import singleton

import os
import sys
import psutil
import time


@singleton
class SingleInstance:
    def acquire(self, retry=False):
        if os.path.exists(lock_file):
            with open(lock_file, "r") as f:
                pid = int(f.read().strip())

            if psutil.pid_exists(pid):
                if not retry:
                    time.sleep(1)
                    self.acquire(True)
                else:
                    sys.exit(1)
            else:
                os.remove(lock_file)

        with open(lock_file, "w") as f:
            f.write(str(os.getpid()))


single_instance = SingleInstance()
