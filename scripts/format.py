import subprocess


def format_code():
    """Format python code"""
    print("Formatting Python code...")
    subprocess.run(["python", "-m", "black", "--line-length", "120", "."], check=False)


if __name__ == "__main__":
    format_code()
