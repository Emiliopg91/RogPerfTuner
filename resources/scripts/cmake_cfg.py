# pylint: disable=invalid-name

from dataclasses import dataclass, field
from enum import Enum, auto
import subprocess

import glob
import os
import re
import shutil

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PROJECT_DIR = os.path.join(BASE_DIR, "RogPerfTuner")
CMAKE_CFG = os.path.join(PROJECT_DIR, "config.cmake")


BOOST_CONTROL_OPTS = {
    "/sys/devices/system/cpu/intel_pstate/no_turbo": {"on": "0", "off": "1"},
    "/sys/devices/system/cpu/cpufreq/boost": {"on": "1", "off": "0"},
}

ACPI_PROFILE_PATH = "/sys/firmware/acpi/platform_profile"
BAT_LIMIT_GLOB = "/sys/class/power_supply/BAT[0-9]*/charge_control_end_threshold"
BAT_STATUS_GLOB = "/sys/class/power_supply/BAT[0-9]*/status"
BOOT_SOUND_PATH = "/sys/class/firmware-attributes/asus-armoury/attributes/boot_sound"
INTEL_RAPL_UJ_GLOB = "/sys/class/powercap/intel-rapl:[0-9]/energy_uj"
NVIDIA_BOOST_PATH = (
    "/sys/class/firmware-attributes/asus-armoury/attributes/nv_dynamic_boost"
)
NVIDIA_THERMAL_PATH = (
    "/sys/class/firmware-attributes/asus-armoury/attributes/nv_temp_target"
)
PANEL_OD_PATH = "/sys/class/firmware-attributes/asus-armoury/attributes/panel_overdrive"
PPT_PL1_SPL_PATH = "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl1_spl"
PPT_PL2_SPPT_PATH = (
    "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl2_sppt"
)
PPT_PL3_FPPT_PATH = (
    "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl3_fppt"
)
SCALING_GOVERNOR_GLOB = "/sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"


class Feature(Enum):
    DEV_MODE = auto()
    ACPI_PROFILE = auto()
    BAT_LIMIT = auto()
    BAT_STATUS = auto()
    BOOST_CONTROL = auto()
    BOOT_SOUND = auto()
    FAN_CONTROL = auto()
    INTEL_RAPL_UJ = auto()
    NV_BOOST = auto()
    NV_THERMAL = auto()
    PANEL_OD = auto()
    PPT_PL1_SPL = auto()
    PPT_PL2_SPPT = auto()
    PPT_PL3_FPPT = auto()
    SCALING_GOVERNOR = auto()


FEATURE_FILES: dict[Feature, list[str]] = {
    Feature.ACPI_PROFILE: [
        "include/clients/file/power_profile_client.hpp",
        "src/clients/file/power_profile_client.cpp",
    ],
    Feature.BAT_LIMIT: [
        "include/clients/file/battery_charge_limit_client.hpp",
        "include/models/hardware/battery_charge_threshold.hpp",
        "src/clients/file/battery_charge_limit_client.cpp",
    ],
    Feature.BAT_STATUS: [
        "include/clients/file/battery_status_client.hpp",
        "src/clients/file/battery_status_client.cpp",
    ],
    Feature.BOOST_CONTROL: [
        "include/clients/file/boost_control_client.hpp",
        "src/clients/file/boost_control_client.cpp",
    ],
    Feature.BOOT_SOUND: [
        "include/clients/file/firmware/asus-armoury/other/boot_sound_client.hpp",
        "src/clients/file/firmware/asus-armoury/other/boot_sound_client.cpp",
    ],
    Feature.FAN_CONTROL: [
        "include/gui/fan_curve_editor.hpp",
        "include/gui/fan_curve_view.hpp",
        "include/models/hardware/fan_curve_data.hpp",
        "include/models/settings/fan_curve.hpp",
        "src/gui/fan_curve_editor.cpp",
        "src/gui/fan_curve_view.cpp",
    ],
    Feature.INTEL_RAPL_UJ: [
        "include/clients/file/intel_rapl_uj_client.hpp",
        "src/clients/file/intel_rapl_uj_client.cpp",
    ],
    Feature.NV_BOOST: [
        "include/clients/file/firmware/asus-armoury/nvidia/nv_boost_client.hpp",
        "src/clients/file/firmware/asus-armoury/nvidia/nv_boost_client.cpp",
    ],
    Feature.NV_THERMAL: [
        "include/clients/file/firmware/asus-armoury/nvidia/nv_temp_client.hpp",
        "src/clients/file/firmware/asus-armoury/nvidia/nv_temp_client.cpp",
    ],
    Feature.PANEL_OD: [
        "include/clients/file/firmware/asus-armoury/other/panel_overdrive_client.hpp",
        "src/clients/file/firmware/asus-armoury/other/panel_overdrive_client.cpp",
    ],
    Feature.PPT_PL1_SPL: [
        "include/clients/file/firmware/asus-armoury/intel/pl1_spd_client.hpp",
        "src/clients/file/firmware/asus-armoury/intel/pl1_spd_client.cpp",
    ],
    Feature.PPT_PL2_SPPT: [
        "include/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.hpp",
        "src/clients/file/firmware/asus-armoury/intel/pl2_sppt_client.cpp",
    ],
    Feature.PPT_PL3_FPPT: [
        "include/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.hpp",
        "src/clients/file/firmware/asus-armoury/intel/pl3_fppt_client.cpp",
    ],
    Feature.SCALING_GOVERNOR: [
        "include/clients/file/scaling_governor_client.hpp",
        "include/models/performance/cpu_governor.hpp",
        "src/clients/file/scaling_governor_client.cpp",
    ],
}


class Definition(Enum):
    ACPI_PROFILE_FILE = auto()
    BAT_LIMIT_FILE = auto()
    BAT_STATUS_FILE = auto()
    BOOT_SOUND_FILE = auto()
    BOOST_CONTROL_FILE = auto()
    BOOST_CONTROL_OFF = auto()
    BOOST_CONTROL_ON = auto()
    INTEL_RAPL_UJ_FILE = auto()
    NVIDIA_BOOST_FILE = auto()
    NVIDIA_THERMAL_FILE = auto()
    PANEL_OD_FILE = auto()
    PPT_PL1_SPL_FILE = auto()
    PPT_PL2_SPPT_FILE = auto()
    PPT_PL3_FPPT_FILE = auto()
    SCALING_GOVERNOR_FILE = auto()
    GPU_BRAND = auto()
    GPU_NAME = auto()
    GPU_ENV = auto()
    CPU_NAME = auto()


feature_definition_asoc: dict[Feature, list[Definition]] = {
    Feature.DEV_MODE: [],
    Feature.ACPI_PROFILE: [Definition.ACPI_PROFILE_FILE],
    Feature.BAT_LIMIT: [Definition.BAT_LIMIT_FILE],
    Feature.BAT_STATUS: [Definition.BAT_STATUS_FILE],
    Feature.BOOT_SOUND: [Definition.BOOT_SOUND_FILE],
    Feature.BOOST_CONTROL: [
        Definition.BOOST_CONTROL_FILE,
        Definition.BOOST_CONTROL_OFF,
        Definition.BOOST_CONTROL_ON,
    ],
    Feature.INTEL_RAPL_UJ: [Definition.INTEL_RAPL_UJ_FILE],
    Feature.NV_BOOST: [Definition.NVIDIA_BOOST_FILE],
    Feature.NV_THERMAL: [Definition.NVIDIA_THERMAL_FILE],
    Feature.PANEL_OD: [Definition.PANEL_OD_FILE],
    Feature.PPT_PL1_SPL: [Definition.PPT_PL1_SPL_FILE],
    Feature.PPT_PL2_SPPT: [Definition.PPT_PL2_SPPT_FILE],
    Feature.PPT_PL3_FPPT: [Definition.PPT_PL3_FPPT_FILE],
    Feature.SCALING_GOVERNOR: [Definition.SCALING_GOVERNOR_FILE],
}

enabled_features: dict[Feature, bool] = {f: False for f in Feature}
definitions: dict[Definition, str] = {
    Definition.ACPI_PROFILE_FILE: "",
    Definition.BAT_LIMIT_FILE: "",
    Definition.BAT_STATUS_FILE: "",
    Definition.BOOT_SOUND_FILE: "",
    Definition.BOOST_CONTROL_FILE: "",
    Definition.BOOST_CONTROL_OFF: "",
    Definition.BOOST_CONTROL_ON: "",
    Definition.INTEL_RAPL_UJ_FILE: "",
    Definition.NVIDIA_BOOST_FILE: "",
    Definition.NVIDIA_THERMAL_FILE: "",
    Definition.PANEL_OD_FILE: "",
    Definition.PPT_PL1_SPL_FILE: "",
    Definition.PPT_PL2_SPPT_FILE: "",
    Definition.PPT_PL3_FPPT_FILE: "",
    Definition.SCALING_GOVERNOR_FILE: "",
}


@dataclass
class GPUInfo:
    name: str = ""
    default_flag: bool = False
    environment: list[str] = field(default_factory=list)


def get_gpus() -> list[GPUInfo]:
    stdout_str = subprocess.run(
        "LANG=C switcherooctl", shell=True, capture_output=True, text=True, check=True
    ).stdout
    gpus = []
    lines = stdout_str.splitlines()

    current_gpu = GPUInfo()
    in_device_block = False

    for line in lines:
        line = line.strip()

        if not line:
            continue

        if line.startswith("Device:"):
            if in_device_block:
                gpus.append(current_gpu)
            current_gpu = GPUInfo()
            in_device_block = True

        elif line.startswith("Name:"):
            current_gpu.name = line[5:].strip()

        elif line.startswith("Default:"):
            val = line[8:].strip()
            current_gpu.default_flag = val == "yes"

        elif line.startswith("Environment:"):
            env = line[12:].strip()
            current_gpu.environment.extend(env.split())

    if in_device_block:
        gpus.append(current_gpu)

    return gpus


if __name__ == "__main__":
    print("Creating CMake config file...")

    print("  Looking for header and source files...")
    header_files = []
    for f in glob.glob(
        os.path.join(PROJECT_DIR, "include", "**", "*.hpp"), recursive=True
    ):
        f = f.replace(PROJECT_DIR + "/", "")
        exclude = False
        for _, files in FEATURE_FILES.items():
            for fi in files:
                if f.endswith(fi):
                    exclude = True
                    break
        if not exclude:
            header_files.append(f)
    header_files = sorted(header_files)

    source_files = []
    for f in glob.glob(os.path.join(PROJECT_DIR, "src", "**", "*.cpp"), recursive=True):
        f = f.replace(PROJECT_DIR + "/", "")
        exclude = False
        for _, files in FEATURE_FILES.items():
            for fi in files:
                if f.endswith(fi):
                    exclude = True
                    break
        if not exclude:
            source_files.append(f)
    source_files = sorted(source_files)

    print("  Checking for features...")
    if os.environ.get("IN_TEST"):
        print("    Test mode active, activating every feature")
        enabled_features = {f: True for f in Feature}
    else:
        if os.environ.get("DEV_MODE"):
            enabled_features[Feature.DEV_MODE] = True
            print("    - Dev mode")

        if os.path.isfile(ACPI_PROFILE_PATH):
            enabled_features[Feature.ACPI_PROFILE] = True
            definitions[Definition.ACPI_PROFILE_FILE] = ACPI_PROFILE_PATH
            print(f"    - ACPI Profiles via {ACPI_PROFILE_PATH}")

        g = glob.glob(BAT_LIMIT_GLOB)
        if len(g) > 0:
            enabled_features[Feature.BAT_LIMIT] = True
            definitions[Definition.BAT_LIMIT_FILE] = g[0]
            print(f"    - Battery limit via {g[0]}")

        g = glob.glob(BAT_STATUS_GLOB)
        if len(g) > 0:
            enabled_features[Feature.BAT_STATUS] = True
            definitions[Definition.BAT_STATUS_FILE] = g[0]
            print(f"    - Battery status via {g[0]}")

        for file, values in BOOST_CONTROL_OPTS.items():
            if os.path.isfile(file):
                enabled_features[Feature.BOOST_CONTROL] = True
                definitions[Definition.BOOST_CONTROL_FILE] = file
                definitions[Definition.BOOST_CONTROL_OFF] = values["off"]
                definitions[Definition.BOOST_CONTROL_ON] = values["on"]
                print(f"    - Boost control via {file}")
                break

        if os.path.isdir(BOOT_SOUND_PATH):
            enabled_features[Feature.BOOT_SOUND] = True
            definitions[Definition.BOOT_SOUND_FILE] = BOOT_SOUND_PATH
            print(f"    - Boot sound via {BOOT_SOUND_PATH}")

        if shutil.which("asusctl"):
            result = subprocess.run(
                ["bash", "-c", "asusctl fan-curve --get-enabled | wc -l"],
                capture_output=True,
                text=True,
                check=True,
            )
            fan_curve_count = int(result.stdout.strip())
            if fan_curve_count > 0:
                enabled_features[Feature.FAN_CONTROL] = True
                print("    - Fan control via asusctl")

        g = glob.glob(INTEL_RAPL_UJ_GLOB)
        if len(g) > 0:
            enabled_features[Feature.INTEL_RAPL_UJ] = True
            definitions[Definition.INTEL_RAPL_UJ_FILE] = g[0]
            print(f"    - Intel Rapl UJ via {g[0]}")

        if os.path.isdir(NVIDIA_BOOST_PATH):
            enabled_features[Feature.NV_BOOST] = True
            definitions[Definition.NVIDIA_BOOST_FILE] = NVIDIA_BOOST_PATH
            print(f"    - Nvidia Boost via {NVIDIA_BOOST_PATH}")

        if os.path.isdir(NVIDIA_THERMAL_PATH):
            enabled_features[Feature.NV_THERMAL] = True
            definitions[Definition.NVIDIA_THERMAL_FILE] = NVIDIA_THERMAL_PATH
            print(f"    - Nvidia Thermal via {NVIDIA_THERMAL_PATH}")

        if os.path.isdir(PANEL_OD_PATH):
            enabled_features[Feature.PANEL_OD] = True
            definitions[Definition.PANEL_OD_FILE] = PANEL_OD_PATH
            print(f"    - Panel overdrive via {PANEL_OD_PATH}")

        if os.path.isdir(PPT_PL1_SPL_PATH):
            enabled_features[Feature.PPT_PL1_SPL] = True
            definitions[Definition.PPT_PL1_SPL_FILE] = PPT_PL1_SPL_PATH
            print(f"    - TDP PL1 SPD via {PPT_PL1_SPL_PATH}")

            if os.path.isdir(PPT_PL2_SPPT_PATH):
                enabled_features[Feature.PPT_PL2_SPPT] = True
                definitions[Definition.PPT_PL2_SPPT_FILE] = PPT_PL2_SPPT_PATH
                print(f"    - TDP PL2 SPPT via {PPT_PL2_SPPT_PATH}")

                if os.path.isdir(PPT_PL3_FPPT_PATH):
                    enabled_features[Feature.PPT_PL3_FPPT] = True
                    definitions[Definition.PPT_PL3_FPPT_FILE] = PPT_PL3_FPPT_PATH
                    print(f"    - TDP PL3 FPPT via {PPT_PL3_FPPT_PATH}")

        g = glob.glob(SCALING_GOVERNOR_GLOB)
        if len(g) > 0:
            enabled_features[Feature.SCALING_GOVERNOR] = True
            definitions[Definition.SCALING_GOVERNOR_FILE] = SCALING_GOVERNOR_GLOB
            print(f"    - Scaling governor via {SCALING_GOVERNOR_GLOB}")

    print("  Detecting CPU...")
    cpu_name = subprocess.run(
        "LANG=C cat /proc/cpuinfo | grep \"model name\" | head -n1 | cut -d':' -f2",
        check=True,
        shell=True,
        capture_output=True,
        text=True,
    ).stdout.strip()
    cpu_name = re.sub(r"\s*\([^)]*\)", "", cpu_name)
    print(f"    Detected {cpu_name}")
    definitions[Definition.CPU_NAME] = cpu_name

    print("  Detecting GPU...")
    gpu_brand = gpu_name = gpu_env = None
    gpus = get_gpus()
    for gpu in gpus:
        if not gpu.default_flag:
            gpu_name = gpu.name.replace("Advanced Micro Devices, Inc.", "AMD")
            gpu_brand = gpu_name.split(" ")[0].lower()
            gpu_env = " ".join(gpu.environment)
            print(f"    Detected {gpu_name}")
            definitions[Definition.GPU_BRAND] = gpu_brand
            definitions[Definition.GPU_NAME] = gpu_name
            definitions[Definition.GPU_ENV] = gpu_env
            break

    print("  Generating config file...")
    print(f"  Writting in {CMAKE_CFG}")

    with open(CMAKE_CFG, "w", encoding="utf-8") as f:
        f.write("set(HEADERS_RCC\n")
        for h in header_files:
            f.write(f'    "${{CMAKE_CURRENT_SOURCE_DIR}}/{h}"\n')
        f.write(")\n")
        f.write("\n")
        f.write("set(SOURCES_RCC\n")
        for h in source_files:
            f.write(f'    "${{CMAKE_CURRENT_SOURCE_DIR}}/{h}"\n')
        f.write(")\n\n")

        for feat in enabled_features:
            if enabled_features[feat]:
                separator = "".ljust(22 + len(feat.name), "#")
                f.write(f"{separator}\n")
                f.write(f"########## {feat.name} ##########\n")
                f.write(f"{separator}\n")
                f.write(
                    f"set({feat.name} {"ON" if enabled_features[feat] else "OFF"})\n"
                )
                if feat in feature_definition_asoc:
                    for deff in feature_definition_asoc[feat]:
                        f.write(
                            f'add_definitions(-D{deff.name}="{definitions[deff]}")\n'
                        )
                if feat in FEATURE_FILES:
                    sources = []
                    headers = []

                    for file in FEATURE_FILES[feat]:
                        if file.startswith("src/"):
                            sources.append(file)
                        elif file.startswith("include/"):
                            headers.append(file)

                    if len(headers) > 0:
                        f.write("list(APPEND HEADERS_RCC\n")
                        for h in headers:
                            f.write(f'    "${{CMAKE_CURRENT_SOURCE_DIR}}/{h}"\n')
                        f.write(")\n")
                    if len(sources) > 0:
                        f.write("list(APPEND SOURCES_RCC\n")
                        for s in sources:
                            f.write(f'    "${{CMAKE_CURRENT_SOURCE_DIR}}/{s}"\n')
                        f.write(")\n")

                f.write("\n")

            if Definition.CPU_NAME in definitions:
                f.write("#########################\n")
                f.write("########## CPU ##########\n")
                f.write("#########################\n")
                f.write(
                    f'add_definitions(-D{Definition.CPU_NAME.name}="{definitions[Definition.CPU_NAME]}")\n'
                )
                f.write("\n")

            if Definition.GPU_BRAND in definitions:
                f.write("#########################\n")
                f.write("########## GPU ##########\n")
                f.write("#########################\n")
                f.write(
                    f'add_definitions(-D{Definition.GPU_BRAND.name}="{definitions[Definition.GPU_BRAND]}")\n'
                )
                f.write(
                    f'add_definitions(-D{Definition.GPU_NAME.name}="{definitions[Definition.GPU_NAME]}")\n'
                )
                f.write(
                    f'add_definitions(-D{Definition.GPU_ENV.name}="{definitions[Definition.GPU_ENV]}")\n'
                )
                f.write("\n")

    print("CMake config file created")
