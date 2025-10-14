import os
import shutil
import sys

import subprocess

QT_PLUGIN_FOLDER = "dist/appimage-fs/usr/lib/qt6/plugins"

os.makedirs("dist/appimage-fs/usr/lib",exist_ok=True)

binaries = set([ "build/assets/OpenRGB/OpenRGB","dist/RogPerfTuner/RogPerfTuner"])
excluded = ["/lib64/ld-linux-x86-64.so.2", "linux-vdso.so.1", "libc.so.6", "libm.so.6", "libgcc_s.so.1"]

plugin_folder = ["/usr/lib/qt6/plugins", "/usr/lib/x86_64-linux-gnu/qt6/plugins", "/usr/lib64/qt6/plugins" ]
for i in range(0,len(plugin_folder)):
    f = plugin_folder[i]
    if os.path.exists(f):
        print(f"Copying QT plugins from {f}")
        shutil.copytree(f, QT_PLUGIN_FOLDER)
        break
    if i==len(plugin_folder)-1:
        print("QT plugins folder not found")
        sys.exit(1)

for plugin in os.listdir(QT_PLUGIN_FOLDER):
    path=os.path.join(QT_PLUGIN_FOLDER,plugin)
    if os.path.isdir(path):
        for so in os.listdir():
            binaries.add(os.path.join(path,so))
    else:
        binaries.add(path)

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