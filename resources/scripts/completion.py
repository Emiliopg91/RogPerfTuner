import os
import re
import textwrap

MAIN_CPP_FILE = os.path.abspath(
    os.path.join(
        os.path.dirname(__file__), "..", "..", "RogPerfTuner", "src", "main.cpp"
    )
)

COMPLETION_BASH_FILE = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "dist", "completion-bash")
)


def __generate_completions():
    with open(MAIN_CPP_FILE, "r", encoding="utf-8") as f:
        content = f.read()

    opciones = [
        o.replace('case hashStr("', "").replace('"):', "")
        for o in re.findall(r'case hashStr\("(--\w+(?:-\w+)*)"\):', content)
    ]
    opciones = [o for o in opciones if o not in ["--run", "--flatpak"]]

    script = textwrap.dedent(
        f"""\
        # /usr/share/bash-completion/completions/rog-perf-tuner
        _rog_perf_tuner() {{
            local cur="${{COMP_WORDS[COMP_CWORD]}}"
            COMPREPLY=( $(compgen -W "{" ".join(opciones)}" -- "$cur") )
        }}
        complete -F _rog_perf_tuner rog-perf-tuner
    """
    )
    with open(COMPLETION_BASH_FILE, "w", encoding="utf-8") as f:
        f.write(script)


if __name__ == "__main__":
    if not os.path.isdir(os.path.dirname(COMPLETION_BASH_FILE)):
        os.makedirs(os.path.dirname(COMPLETION_BASH_FILE))
    __generate_completions()
