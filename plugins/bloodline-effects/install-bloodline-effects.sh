#!/bin/sh
# Installs the Bloodline Effects spike from the unzipped CI download, for this user only, and clears the
# quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-bloodline-effects.sh
# It also removes the copy installed under its old name, "FX Breeder", which has the same plugin ID and
# would clash with this one in Live.
set -e
cd "$(dirname "$0")"
VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components
mkdir -p "$VST3" "$AU"
rm -rf "$VST3/FX Breeder.vst3" "$AU/FX Breeder.component" "$VST3/Bloodline Effects.vst3" "$AU/Bloodline Effects.component"
cp -R "Bloodline Effects.vst3" "$VST3/"
cp -R "Bloodline Effects.component" "$AU/"
xattr -dr com.apple.quarantine "$VST3/Bloodline Effects.vst3" || true
xattr -dr com.apple.quarantine "$AU/Bloodline Effects.component" || true
echo "Installed Bloodline Effects. In Live: Preferences, Plug-Ins, Rescan."
