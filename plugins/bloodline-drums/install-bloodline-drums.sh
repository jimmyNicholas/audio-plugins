#!/bin/sh
# Installs the Bloodline Drums spike from the unzipped CI download, for this user only, and clears the
# quarantine flag a browser download adds (the plugin is not signed). Run from the unzipped folder:
#   sh install-bloodline-drums.sh
# It also removes the first spike, "Bloodline", wherever it is installed (your Library or the system
# one), and resets the Audio Unit cache so Live stops seeing it.
set -e
cd "$(dirname "$0")"
VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components

for dir in ~/Library/Audio/Plug-Ins /Library/Audio/Plug-Ins; do
  for old in "$dir/VST3/Bloodline.vst3" "$dir/Components/Bloodline.component" "$dir/VST/Bloodline.vst"; do
    if [ -e "$old" ]; then
      if rm -rf "$old" 2>/dev/null; then echo "Removed $old"
      else echo "Removing $old needs your password:"; sudo rm -rf "$old" && echo "Removed $old"; fi
    fi
  done
done

mkdir -p "$VST3" "$AU"
rm -rf "$VST3/Bloodline Drums.vst3" "$AU/Bloodline Drums.component"
cp -R "Bloodline Drums.vst3" "$VST3/"
cp -R "Bloodline Drums.component" "$AU/"
xattr -dr com.apple.quarantine "$VST3/Bloodline Drums.vst3" || true
xattr -dr com.apple.quarantine "$AU/Bloodline Drums.component" || true

# macOS keeps a cache of Audio Units; this makes it look again.
killall -9 AudioComponentRegistrar 2>/dev/null || true
rm -f ~/Library/Caches/AudioUnitCache/com.apple.audiounits.cache 2>/dev/null || true

echo "Installed Bloodline Drums. Quit and reopen Live, then Preferences, Plug-Ins, Rescan."
