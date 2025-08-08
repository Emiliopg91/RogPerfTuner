SRC_PATH="$1"
DST_PATH="$2"

if [[ -d "$2" ]]; then
    rm -R "$2"
fi
cp -R "$1" "$2"
systemctl restart plugin_loader.service
