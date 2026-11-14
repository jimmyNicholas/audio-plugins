#!/bin/sh
# PROTOTYPE. Installs the Bloodline Breaks spike from the unzipped CI download, for this user only, and clears
# the quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-bloodline-breaks.sh
set -e
cd "$(dirname "$0")"
VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components
mkdir -p "$VST3" "$AU"
rm -rf "$VST3/Bloodline Breaks.vst3" "$AU/Bloodline Breaks.component"
cp -R "Bloodline Breaks.vst3" "$VST3/"
cp -R "Bloodline Breaks.component" "$AU/"
xattr -dr com.apple.quarantine "$VST3/Bloodline Breaks.vst3" || true
xattr -dr com.apple.quarantine "$AU/Bloodline Breaks.component" || true
echo "Installed Bloodline Breaks. In Live: Preferences, Plug-Ins, Rescan."
