#!/bin/sh
# Installs the Bloodline Drums spike from the unzipped CI download, for this user only, and clears the
# quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-bloodline-drums.sh
# It also removes the copy installed under its old name, "Bloodline", which has the same plugin ID and
# would clash with this one in Live.
set -e
cd "$(dirname "$0")"
VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components
mkdir -p "$VST3" "$AU"
rm -rf "$VST3/Bloodline.vst3" "$AU/Bloodline.component" "$VST3/Bloodline Drums.vst3" "$AU/Bloodline Drums.component"
cp -R "Bloodline Drums.vst3" "$VST3/"
cp -R "Bloodline Drums.component" "$AU/"
xattr -dr com.apple.quarantine "$VST3/Bloodline Drums.vst3" || true
xattr -dr com.apple.quarantine "$AU/Bloodline Drums.component" || true
echo "Installed Bloodline Drums. In Live: Preferences, Plug-Ins, Rescan."
