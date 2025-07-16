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

"""
LOGFILE = "{{log_file}}"
"""


# === Configuración del logger ===
class CustomFormatter(logging.Formatter):
    def formatTime(self, record, datefmt=None):
        dt = datetime.fromtimestamp(record.created)
        return dt.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


log_formatter = CustomFormatter("[%(asctime)s] %(message)s")

logger = logging.getLogger("launcher")
logger.setLevel(logging.INFO)

"""
# Handler a archivo
file_handler = logging.FileHandler(LOGFILE, encoding="utf-8", mode="w")
file_handler.setFormatter(log_formatter)
logger.addHandler(file_handler)
"""

# Handler a consola
console_handler = logging.StreamHandler()
console_handler.setFormatter(log_formatter)
logger.addHandler(console_handler)

# === Inicio del script ===

if len(sys.argv) < 2:
    logger.error("Error: no command provided")
    sys.exit(1)

logger.info("===== Lanzamiento iniciado =====")
logger.info(">>> Entorno inicial:")
for key, value in os.environ.items():
    logger.info("\t%s=%s", key, value)

app_id = None

logger.info(">>> Argumentos originales:")
for arg in sys.argv[1:]:
    logger.info("\t%s", arg)
    if arg.startswith("AppId="):
        app_id = arg.split("=")[1]


child_env = os.environ.copy()
command = sys.argv[1:]

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
        env_vars = config.get("environment", [])
        wrappers = config.get("wrappers", [])

        logger.info(">>> Configuración descargada:")
        logger.info("\tEnvironment:")

        for pair in env_vars:
            if "=" in pair:
                key, value = pair.split("=", 1)
                logger.info("\t\t%s=%s", key, value)
                child_env[key] = value

        logger.info("\tWrappers:")
        for wrapper in wrappers:
            logger.info("\t\t%s", wrapper)
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

logger.info(">>> Running command: %s", " ".join(command))
start_time = time.time()
try:
    result = subprocess.run(command, env=child_env, check=False)
    exit_code = result.returncode
except FileNotFoundError:
    exit_code = 127
    logger.error("Command not found: %s", command[0])
except Exception as e:
    exit_code = 1
    logger.error("Error running command: %s", e)

end_time = time.time()

logger.info(">>> Finished after %f", (end_time - start_time))
logger.info(">>> Exit code: %d", exit_code)

sys.exit(exit_code)
