#!/bin/bash
UPDATE_PATH="{{update}}"
APPIMAGE_PATH="{{appimage}}"

if [[ -f "$UPDATE_PATH" ]]; then
    cp "$UPDATE_PATH" "$APPIMAGE_PATH"
    chmod 755 "$APPIMAGE_PATH"
    rm "$UPDATE_PATH"
fi
"{{appimage}}"