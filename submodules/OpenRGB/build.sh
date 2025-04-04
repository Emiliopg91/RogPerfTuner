podman build -t openrgb-build .
rm -R ./build
mkdir build
podman run --rm --privileged --cpus=$(nproc) -v $(pwd):/input openrgb-build
if [ -f ./build/openrgb ]; then
    /var/mnt/Datos/Desarrollo/Workspace/VSCode/RogControlCenter/assets/OpenRGB.AppImage --appimage-extract
    cp ./build/openrgb squashfs-root/usr/bin/OpenRGB
    /var/mnt/Datos/Desarrollo/Workspace/VSCode/AppImage-Creator-2/resources/appimagetool squashfs-root/ ./OpenRGB-Exp.AppImage
fi