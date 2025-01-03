import os
import signal
import subprocess


def run_pylint():
    """Lint Python code"""
    print("Linting Python code...")

    pylint_command = [
        "python",
        "-m",
        "pylint",
        f"--rcfile={os.path.join(".",".pylintrc")}",
        ".",
    ]
    result = subprocess.run(pylint_command, check=False)

    if result.returncode != 0:
        print(result.stderr)
        print("Linting failed")
        os.kill(os.getpid(), signal.SIGKILL)


if __name__ == "__main__":
    run_pylint()
