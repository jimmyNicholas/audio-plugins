#!/bin/sh
# PROTOTYPE. Installs the Bloodline Breaks 2 spike from the unzipped CI download, for this user only, and clears
# the quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-bloodline-breaks-2.sh
set -e
cd "$(dirname "$0")"
VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components
mkdir -p "$VST3" "$AU"
rm -rf "$VST3/Bloodline Breaks 2.vst3" "$AU/Bloodline Breaks 2.component"
cp -R "Bloodline Breaks 2.vst3" "$VST3/"
cp -R "Bloodline Breaks 2.component" "$AU/"
xattr -dr com.apple.quarantine "$VST3/Bloodline Breaks 2.vst3" || true
xattr -dr com.apple.quarantine "$AU/Bloodline Breaks 2.component" || true
echo "Installed Bloodline Breaks 2. In Live: Preferences, Plug-Ins, Rescan."
