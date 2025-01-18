import json
import os
import subprocess
import sys
import threading
import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler


def get_pylint_args() -> list[str]:
    """Obtiene el valor de la propiedad `python.linting.pylintArgs` del settings.json.

    Returns:
        list[str] | None: Un array de strings si existe, o None si no existe.
    """
    settings_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".vscode", "settings.json"))

    # Verificar si el archivo existe
    if not os.path.exists(settings_path):
        print(f"No such file: {settings_path}")
        return None

    try:
        # Leer el contenido del archivo
        with open(settings_path, "r", encoding="utf-8") as file:
            settings = json.load(file)

        # Devolver el valor de la propiedad o None si no existe
        return settings.get("python.linting.pylintArgs", [])

    except json.JSONDecodeError as e:
        print(f"Error while reading settings.json: {e}")
        return None


def run_pylint():
    """Lint Python code"""
    print("Linting Python code...")

    pylint_command = [
        "python",
        "-m",
        "pylint",
    ]
    for _, line in enumerate(get_pylint_args()):
        for _, param in enumerate(line.replace("${workspaceFolder}", ".").split(" ")):
            pylint_command.append(param)

    result = subprocess.run(pylint_command, check=False)

    if result.returncode != 0:
        print("Linting failed")
        return False

    print("Linting passed!")
    sys.exit(0)


class FileChangeHandler(FileSystemEventHandler):
    """Handler to detect file changes"""

    def __init__(self, stop_event):
        self.stop_event = stop_event

    def on_modified(self, event):
        """Trigger event when a Python file is modified"""
        if event.src_path.endswith(".py"):
            print("File(s) modified")
            self.stop_event.set()


def wait_for_changes_and_lint():
    """Wait for file changes and rerun pylint until successful"""
    stop_event = threading.Event()

    # Set up file system observer
    event_handler = FileChangeHandler(stop_event)
    observer = Observer()
    observer.schedule(event_handler, path=".", recursive=True)
    observer.start()

    try:
        # Run pylint initially
        if not run_pylint():
            print("Waiting for file changes to rerun linting...")

        # Keep running pylint when there are file changes
        while True:
            stop_event.wait()  # Wait for a file modification event
            stop_event.clear()  # Reset the event flag

            # Rerun pylint after a file change
            if run_pylint():
                break  # Exit if pylint passes

            # If linting fails again, keep waiting for more changes
            print("Waiting for file changes to rerun linting...")

    except KeyboardInterrupt:
        print("Monitoring interrupted. Exiting...")
    finally:
        observer.stop()
        observer.join()


if __name__ == "__main__":
    wait_for_changes_and_lint()
