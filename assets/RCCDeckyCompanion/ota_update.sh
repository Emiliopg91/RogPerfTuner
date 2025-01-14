#!/usr/bin/bash
# does the following:
# - Update AllyDeckyCompanion Decky Plugin

if [ ! -f '/tmp/AllyDeckyCompanion.tar.gz' ]; then
  echo "Failed to find downloaded plugin"
  exit -1
fi

DECKY_DIR="$HOME/homebrew/plugins"

if [ ! -d $DECKY_DIR ]; then
  echo "Failed to find DECKY_DIR at: " 
  echo $DECKY_DIR
  exit -1
fi

rm -rf $DECKY_DIR/AllyDeckyCompanion

tar -xzf /tmp/AllyDeckyCompanion.tar.gz -C $DECKY_DIR

# install complete, remove files
rm  -rf /tmp/AllyDeckyCompanion.tar.gz

systemctl restart plugin_loader.service