from .constants import lock_file
from .singleton import singleton

import os
import sys
import psutil


@singleton
class SingleInstance:
    def acquire(self):
        if os.path.exists(lock_file):
            with open(lock_file, "r") as f:
                pid = int(f.read().strip())

            if psutil.pid_exists(pid):
                sys.exit(1)
            else:
                os.remove(lock_file)

        with open(lock_file, "w") as f:
            f.write(str(os.getpid()))


single_instance = SingleInstance()
