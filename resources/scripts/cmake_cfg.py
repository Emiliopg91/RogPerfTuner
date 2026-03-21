# pylint: disable=invalid-name

from enum import Enum, auto
import subprocess

import glob
import os
import shutil

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
PROJECT_DIR = os.path.join(BASE_DIR, "RogPerfTuner")
CMAKE_CFG = os.path.join(PROJECT_DIR, "config.cmake")


BOOST_CONTROL_OPTS = {
    "/sys/devices/system/cpu/intel_pstate/no_turbo": {"on": "0", "off": "1"},
    "/sys/devices/system/cpu/cpufreq/boost": {"on": "1", "off": "0"},
}


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


FEATURE_FILES: dict[Feature, list[str]] = {
    Feature.ACPI_PROFILE: [
        "include/clients/file/power_profile_client.hpp",
        "src/clients/file/power_profile_client.cpp",
    ],
    Feature.BAT_LIMIT: [
        "include/clients/file/battery_charge_limit_client.hpp",
        "include/models/hardware/battery_charge_threshold.hpp"
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
}

enabled_features: dict[Feature, bool] = {f: False for f in Feature}
definitions: dict[str, str] = {
    "BOOST_CONTROL_FILE": "",
    "BOOST_CONTROL_OFF": "",
    "BOOST_CONTROL_ON": "",
}

if __name__ == "__main__":
    print("Creating CMake config file...")

    print("  Looking for header and source files...")
    header_files = [
        f.replace(PROJECT_DIR + "/", "")
        for f in glob.glob(
            os.path.join(PROJECT_DIR, "include", "**", "*.hpp"), recursive=True
        )
    ]
    header_files = sorted(header_files)

    source_files = [
        f.replace(PROJECT_DIR + "/", "")
        for f in glob.glob(
            os.path.join(PROJECT_DIR, "src", "**", "*.cpp"), recursive=True
        )
    ]
    source_files = sorted(source_files)

    print("  Checking for features...")
    if os.environ.get("IN_TEST"):
        print("    Test mode active, activating every feature")
        enabled_features = {f: True for f in Feature}
    else:
        if os.environ.get("DEV_MODE"):
            enabled_features[Feature.DEV_MODE] = True

        if os.path.isfile("/sys/firmware/acpi/platform_profile"):
            enabled_features[Feature.ACPI_PROFILE] = True

        if os.path.isfile("/sys/class/power_supply/BAT0/charge_control_end_threshold"):
            enabled_features[Feature.BAT_LIMIT] = True

        if os.path.isfile("/sys/class/power_supply/BAT0/status"):
            enabled_features[Feature.BAT_STATUS] = True

        for file, values in BOOST_CONTROL_OPTS.items():
            if os.path.isfile(file):
                enabled_features[Feature.BOOST_CONTROL] = True
                definitions["BOOST_CONTROL_FILE"] = file
                definitions["BOOST_CONTROL_OFF"] = values["off"]
                definitions["BOOST_CONTROL_ON"] = values["on"]
                break

        if os.path.isfile(
            "/sys/class/firmware-attributes/asus-armoury/attributes/boot_sound/current_value"
        ):
            enabled_features[Feature.BOOT_SOUND] = True

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

        if os.path.isfile("/sys/class/powercap/intel-rapl:0/energy_uj"):
            enabled_features[Feature.INTEL_RAPL_UJ] = True

        if os.path.isfile(
            "/sys/class/firmware-attributes/asus-armoury/attributes/nv_dynamic_boost/current_value"
        ):
            enabled_features[Feature.NV_BOOST] = True

        if os.path.isfile(
            "/sys/class/firmware-attributes/asus-armoury/attributes/nv_temp_target/current_value"
        ):
            enabled_features[Feature.NV_THERMAL] = True

        if os.path.isfile(
            "/sys/class/firmware-attributes/asus-armoury/attributes/panel_overdrive/current_value"
        ):
            enabled_features[Feature.PANEL_OD] = True

        if os.path.isfile(
            "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl1_spl/current_value"
        ):
            enabled_features[Feature.PPT_PL1_SPL] = True
            if os.path.isfile(
                "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl2_sppt/current_value"
            ):
                enabled_features[Feature.PPT_PL2_SPPT] = True
                if os.path.isfile(
                    "/sys/class/firmware-attributes/asus-armoury/attributes/ppt_pl3_fppt/current_value"
                ):
                    enabled_features[Feature.PPT_PL3_FPPT] = True

    print("  Removing unused files...")
    for feature, enabled in enabled_features.items():
        if not enabled and feature in FEATURE_FILES:
            for file in FEATURE_FILES[feature]:
                removed = False
                if file in header_files:
                    header_files.remove(file)
                    removed = True
                elif file in source_files:
                    source_files.remove(file)
                    removed = True
                if removed:
                    print(f"    Removed {file}")
                else:
                    print(f"    Missing file {file}")

    print("  Generating config file...")
    print(f"  Writting in {CMAKE_CFG}")

    with open(CMAKE_CFG, "w", encoding="utf-8") as f:
        f.write("set(HEADERS_RCC\n")
        for h in header_files:
            f.write(f'      "${{CMAKE_CURRENT_SOURCE_DIR}}/{h}"\n')
        f.write(")\n")
        f.write("\n")
        f.write("set(SOURCES_RCC\n")
        for h in source_files:
            f.write(f'      "${{CMAKE_CURRENT_SOURCE_DIR}}/{h}"\n')
        f.write(")\n")

        for deff, value in definitions.items():
            f.write(f'add_definitions(-D{deff}="{value}")\n')
        f.write("\n")
        for feat in enabled_features:
            f.write(f"set({feat.name} {"ON" if enabled_features[feat] else "OFF"})\n")
    print("CMake config file created")
