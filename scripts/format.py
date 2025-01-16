import json
import os
import subprocess


def get_black_formatter_args():
    """Obtiene el valor de la propiedad `black-formatter.args` del settings.json.

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
        return settings.get("black-formatter.args")

    except json.JSONDecodeError as e:
        print(f"Error while reading settings.json: {e}")
        return None


def format_code():
    """Format python code"""
    print("Formatting Python code...")
    command = ["python", "-m", "black"]

    cfg = get_black_formatter_args()
    if cfg is not None:
        for param in cfg:
            command.append(param)

    command.append(".")
    subprocess.run(command, check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


if __name__ == "__main__":
    format_code()
