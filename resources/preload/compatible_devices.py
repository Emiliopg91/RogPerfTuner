import os
import re
import sys

input_file = os.path.abspath(
    os.path.dirname(__file__)
    + "/../../assets/OpenRGB/usr/lib/udev/rules.d/60-openrgb.rules"
)
output_file = os.path.abspath(
    os.path.dirname(__file__)
    + "/../../RogControlCenter/include/clients/tcp/open_rgb/compatible_devices.hpp"
)

print(f"Preloading compatibles devices from {input_file}")

if os.path.exists(output_file):
    mtime1 = os.path.getmtime(input_file)
    mtime2 = os.path.getmtime(output_file)

    if mtime1 <= mtime2:
        print("    Preload up to date")
        sys.exit(0)

# Regex para extraer vendor, product y nombre
regex = re.compile(
    r'SUBSYSTEMS==".*?", ATTRS\{idVendor\}=="([0-9a-fA-F]+)", ATTRS\{idProduct\}=="([0-9a-fA-F]+)".*?TAG\+="([a-zA-Z0-9_]+)"'
)

devices = []

# Leer y parsear el fichero
with open(input_file) as f:
    for line in f:
        line = line.replace(', TAG+="uaccess"', "")
        m = regex.search(line)
        if m:
            vendor_id, product_id, name = m.groups()
            name = name.replace("_", " ")
            devices.append((vendor_id, product_id, name))

devices = [(v, p, n) for v, p, n in devices if v == "0b05"]
devices.sort(key=lambda x: x[2])

constants: dict[str, str] = {}

for dev in devices:
    base_name = dev[2].upper().replace(" ", "_")
    name = base_name

    it = 1
    while name in constants.keys():
        name = base_name + f"_{it}"
        it = it + 1
    constants[name] = f'UsbIdentifier{{"{dev[0]}", "{dev[1]}", "{dev[2]}"}}'

with open(output_file, "w") as out:
    out.write("#pragma once\n\n")

    out.write("#include <array>\n")
    out.write("#include <map>\n")
    out.write("#include <string>\n")
    out.write("#include <string_view>\n\n")

    out.write('#include "../../../models/hardware/usb_identifier.hpp"\n\n')

    for c in constants:
        out.write(f"const auto {c} = {constants[c]};\n")
    out.write("\n")

    out.write(
        f"using CompatibleDeviceArray = std::array<UsbIdentifier, {len(devices)}>;\n"
    )
    out.write("const CompatibleDeviceArray compatibleDevices = {\n")
    for c in constants:
        out.write(f"    {c},\n")
    out.write("};\n\n")

    out.write("using CompatibleDeviceNameMap = std::map<std::string, std::string>;\n")
    out.write("const CompatibleDeviceNameMap compatibleDeviceNameMap = {\n")
    for c in constants:
        out.write(
            f'    {{std::string({c}.id_vendor)+":"+std::string({c}.id_product), std::string({c}.name)}},\n'
        )
    out.write("};\n")

print(f"    Preloaded {len(devices)} devices in {output_file}")
