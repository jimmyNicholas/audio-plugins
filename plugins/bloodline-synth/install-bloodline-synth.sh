#!/bin/sh
# Installs the Bloodline Synth spike from the unzipped CI download, for this user only, and clears the
# quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-bloodline-synth.sh
# It also removes the copy installed under its old name, "Synth Breeder", which has the same plugin ID and
# would clash with this one in Live.
set -e
cd "$(dirname "$0")"
VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components
mkdir -p "$VST3" "$AU"
rm -rf "$VST3/Synth Breeder.vst3" "$AU/Synth Breeder.component" "$VST3/Bloodline Synth.vst3" "$AU/Bloodline Synth.component"
cp -R "Bloodline Synth.vst3" "$VST3/"
cp -R "Bloodline Synth.component" "$AU/"
xattr -dr com.apple.quarantine "$VST3/Bloodline Synth.vst3" || true
xattr -dr com.apple.quarantine "$AU/Bloodline Synth.component" || true
echo "Installed Bloodline Synth. In Live: Preferences, Plug-Ins, Rescan."
