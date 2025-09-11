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

print("Clean up of rules file")
lines=[]
with open(input_file) as f:
    for line in f:
        if len(line.strip())>0 and not line.startswith("#") and ( not line.startswith("SUBSYSTEMS==") or "ATTRS{idVendor}==\"0b05\"" in line):
            lines.append(line.strip())

with open(input_file,"w") as f:
    f.write("\n".join(lines))

regex = re.compile(
    r'SUBSYSTEMS==".*?", ATTRS\{idVendor\}=="([0-9a-fA-F]+)", ATTRS\{idProduct\}=="([0-9a-fA-F]+)".*?TAG\+="([a-zA-Z0-9_]+)"'
)

devices=[]

with open(input_file) as f:
    for line in f:
        line = line.replace(', TAG+="uaccess"', "")
        m = regex.search(line)
        if m:
            vendor_id, product_id, name = m.groups()
            name = name.replace("_", " ")
            devices.append((vendor_id, product_id, name))

with open(output_file, "w") as out:
    out.write("#pragma once\n\n")

    out.write("#include <array>\n")

    out.write('#include "../../../models/hardware/usb_identifier.hpp"\n\n')

    out.write(
        f"using CompatibleDeviceArray = std::array<UsbIdentifierView, {len(devices)}>;\n"
    )
    out.write("const constexpr CompatibleDeviceArray compatibleDevices = {{\n")
    for c in devices:
        out.write(f"    {{\"{c[0]}\",\"{c[1]}\",\"{c[2]}\"}},\n")
    out.write("}};\n\n")