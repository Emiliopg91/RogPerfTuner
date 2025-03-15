import os
import re
import subprocess


cmd_line = "ujust benchmark"

pattern = re.compile(r'([A-Za-z_][A-Za-z0-9_]*)=(?:"([^"]*)"|\'([^\']*)\'|(\S+))')


env_vars = {}
pos = 0
while pos < len(cmd_line):
    match = pattern.match(cmd_line, pos)
    if match:
        key = match.group(1)
        value = match.group(2) or match.group(3) or match.group(4)
        env_vars[key] = value
        pos = match.end()
        # Avanza ignorando espacios después de cada variable
        while pos < len(cmd_line) and cmd_line[pos].isspace():
            pos += 1
    else:
        # El resto es el comando y sus argumentos
        user = os.getenv("USER")
        cmd_line = f"sudo nice -n -10 setpriv --reuid={user} --regid={user} --init-groups --reset-env {cmd_line[pos:]}"
        break

print(f"{env_vars}")
print(cmd_line)

current_env = os.environ.copy()
current_env.update(env_vars)

process = subprocess.run(cmd_line, env=current_env, text=True, capture_output=True, shell=True)
