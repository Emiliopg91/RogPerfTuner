import subprocess
import time
import os
import threading

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

workspace = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))


class ChangeHandler(FileSystemEventHandler):
    """Change handler class"""

    def __init__(self, restart_callback, debounce_interval=1):
        super().__init__()
        self.restart_callback = restart_callback
        self.debounce_interval = debounce_interval
        self._last_modified = None
        self._timer = None

    def _reset_timer(self):
        """Reset debounce timer."""
        if self._timer:
            self._timer.cancel()
        self._timer = threading.Timer(self.debounce_interval, self.restart_callback)
        self._timer.start()

    def on_modified(self, event):
        if event.src_path.endswith(".py"):
            self._reset_timer()

    def on_created(self, event):
        if event.src_path.endswith(".py"):
            self._reset_timer()

    def on_deleted(self, event):
        if event.src_path.endswith(".py"):
            self._reset_timer()


def run_main():
    """Ejecuta el archivo main.py como un subproceso."""
    return subprocess.Popen(["python3", os.path.join(workspace, "main.py")])


if __name__ == "__main__":
    paths = [os.path.join(workspace, "main.py"), os.path.join(workspace, "lib")]
    MAIN_PROCESS = None

    def restart_main():
        """Reinicia el subproceso que ejecuta main.py."""
        global MAIN_PROCESS  # pylint: disable=W0603
        if MAIN_PROCESS:
            print("Changes detected, restarting main.py...")
            MAIN_PROCESS.terminate()
            MAIN_PROCESS.wait()
            time.sleep(1)
        MAIN_PROCESS = run_main()

    # Inicializar el proceso al inicio
    MAIN_PROCESS = run_main()

    # Configurar Watchdog
    event_handler = ChangeHandler(restart_callback=restart_main)
    observers = []
    for path in paths:
        observer = Observer()
        observer.schedule(event_handler, path, recursive=True)
        observer.start()
        observers.append(observer)

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        for observer in observers:
            observer.stop()
        for observer in observers:
            observer.join()
        if MAIN_PROCESS:
            MAIN_PROCESS.terminate()
            MAIN_PROCESS.wait()
