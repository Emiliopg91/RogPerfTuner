#!/usr/bin/env python3
import os
import sys
import json
import subprocess
import logging
from datetime import datetime
import time
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError

LOGGER_FILE = os.path.join("{{logger_path}}", "runner.log")


def format_time(segundos) -> str:
    segundos = round(segundos)
    horas, resto = divmod(segundos, 3600)
    minutos, segundos = divmod(resto, 60)
    return f"{horas:02}:{minutos:02}:{segundos:02}"


# === Configuración del logger ===
class CustomFormatter(logging.Formatter):
    def formatTime(self, record, datefmt=None):
        dt = datetime.fromtimestamp(record.created)
        return dt.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


log_formatter = CustomFormatter("[%(asctime)s] %(message)s")

logger = logging.getLogger("launcher")
logger.setLevel(logging.INFO)

# Handler a archivo
file_handler = logging.FileHandler(LOGGER_FILE, encoding="utf-8", mode="w")
file_handler.setFormatter(log_formatter)
logger.addHandler(file_handler)

# Handler a consola
console_handler = logging.StreamHandler()
console_handler.setFormatter(log_formatter)
logger.addHandler(console_handler)

# === Inicio del script ===

if len(sys.argv) < 2:
    logger.error("Error: no command provided")
    sys.exit(1)

logger.info("===== Lanzamiento iniciado =====")
logger.info(">>> Entorno:")
logger.info("\t%d entries", len(os.environ))

app_id = None

logger.info(">>> Command:")
logger.info("\t%s", " ".join(sys.argv[1:]))


child_env = os.environ.copy()
command = sys.argv[1:]

for p in command[1:]:
    if p.startswith("AppId="):
        app_id = p.split("=")[1]
        break

if app_id is not None:
    try:
        env_vars = []
        wrappers = []

        logger.info("Sending config query for AppId %s", app_id)
        API_URL = f"http://localhost:{{port}}/getParamRunningApp/{app_id}"
        req = Request(
            API_URL,
            headers={"Accept": "application/json"},
        )
        with urlopen(req) as response:
            config = json.load(response)
        env_vars = config.get("environment", {})
        wrappers = config.get("wrappers", [])

        logger.info(">>> Configuración descargada:")
        logger.info("\tEnvironment:")
        logger.info("\t\t%s", env_vars)

        for key in env_vars:
            child_env[key] = env_vars[key]

        logger.info("\tWrappers:")
        logger.info("\t\t%s", " ".join(wrappers))
        command = wrappers + sys.argv[1:]
    except (URLError, HTTPError) as e:
        logger.warning("Error fetching config from %s: %s", API_URL, e)
    except Exception as e:
        logger.warning("Unexpected error during config fetch: %s", e)
else:
    logger.warning("No AppId provided")

try:
    logger.info("Request renice for %s", os.getpid())
    API_URL = f"http://localhost:{{port}}/renice/{os.getpid()}"
    req = Request(
        API_URL,
        headers={"Accept": "application/json"},
    )
    with urlopen(req) as response:
        result = json.load(response)

except (URLError, HTTPError) as e:
    logger.warning("Error on renice from %s: %s", API_URL, e)
except Exception as e:
    logger.warning("Unexpected error during renice: %s", e)

logger.info(">>> Running command: ")
logger.info("\t%s", " ".join(command))
start_time = time.time()
try:
    with open(LOGGER_FILE, "a") as log_file:
        result = subprocess.run(command, env=child_env, check=False, stdout=log_file, stderr=subprocess.STDOUT)
        exit_code = result.returncode
except FileNotFoundError:
    exit_code = 127
    logger.error("Command not found: %s", command[0])
except Exception as e:
    exit_code = 1
    logger.error("Error running command: %s", e)

end_time = time.time()

logger.info(">>> Finished after %s", format_time(end_time - start_time))
logger.info(">>> Exit code: %d", exit_code)

sys.exit(exit_code)
