IS_UPDATE="$1"

cp -R "$2" "$3"
chmod -R 777 "$3"
if [[ "$IS_UPDATE" -eq 1 ]]; then
    rm -R "$4"
    cp -R "$5" "{$4}"
fi
systemctl restart plugin_loader.service