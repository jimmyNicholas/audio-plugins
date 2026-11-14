#!/bin/sh
# Installs the Bloodline spike from the unzipped CI download, for this user only, and clears the
# quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-mac.sh
set -e
cd "$(dirname "$0")"
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
rm -rf ~/Library/Audio/Plug-Ins/VST3/Bloodline.vst3 ~/Library/Audio/Plug-Ins/Components/Bloodline.component
cp -R Bloodline.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R Bloodline.component ~/Library/Audio/Plug-Ins/Components/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Bloodline.vst3 || true
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/Bloodline.component || true
echo "Installed. In Live: Preferences, Plug-Ins, Rescan."
