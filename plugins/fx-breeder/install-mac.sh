#!/bin/sh
# Installs the FX Breeder spike from the unzipped CI download, for this user only, and clears the
# quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-mac.sh
set -e
cd "$(dirname "$0")"
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
rm -rf ~/Library/Audio/Plug-Ins/VST3/"FX Breeder.vst3" ~/Library/Audio/Plug-Ins/Components/"FX Breeder.component"
cp -R "FX Breeder.vst3" ~/Library/Audio/Plug-Ins/VST3/
cp -R "FX Breeder.component" ~/Library/Audio/Plug-Ins/Components/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"FX Breeder.vst3" || true
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"FX Breeder.component" || true
echo "Installed. In Live: Preferences, Plug-Ins, Rescan."
