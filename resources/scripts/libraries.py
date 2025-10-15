import os
import shutil
import sys

import subprocess


os.makedirs("dist/appimage-fs/usr/lib",exist_ok=True)

binaries = set([ "build/assets/OpenRGB/OpenRGB","dist/RogPerfTuner/RogPerfTuner"])
excluded = ["/lib64/ld-linux-x86-64.so.2", "linux-vdso.so.1", "libc.so.6", "libm.so.6", "libgcc_s.so.1"]

for binary in binaries:
    output=subprocess.run(f"ldd {binary} | sort", shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.strip()

    if len(output)>0 and "not found" not in output:
        print(f"Libaries for binary {binary}:")

        for line in output.splitlines():
            line=line.strip()

            if len(line)==0:
                continue

            parts = line.split(" ")
            if len(parts)<3:
                continue

            libName=parts[0]
            libPath=parts[2]

            if libName not in excluded and not os.path.exists(os.path.join("dist/appimage-fs/usr/lib", os.path.basename(libName))):
                print(f"  {libName}")
                shutil.copy2(libPath, "dist/appimage-fs/usr/lib")

for e in os.listdir("dist/appimage-fs/usr/share/rog-perf-tuner/OpenRGB/usr/lib"):
    if os.path.basename(e) != "udev":
        os.unlink(f"dist/appimage-fs/usr/share/rog-perf-tuner/OpenRGB/usr/lib/{e}")
shutil.rmtree("dist/appimage-fs/usr/share/rog-perf-tuner/OpenRGB/usr/plugins")