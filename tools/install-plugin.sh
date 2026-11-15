#!/bin/sh
# Installs the latest green Mac build of a Bloodline plugin, straight from CI, for this user only.
# Needs GitHub's command-line tool, logged in once (gh auth login). Run from any folder:
#   sh tools/install-plugin.sh bloodline-breaks-2
# or, with no copy of the repo, fetch and run it in one go:
#   gh api -H "Accept: application/vnd.github.raw" "repos/jimmyNicholas/audio-plugins/contents/tools/install-plugin.sh?ref=claude/plugin-album-collection-5nvyi5" | sh -s bloodline-breaks-2
set -e

REPO=jimmyNicholas/audio-plugins
BRANCH=${BLOODLINE_BRANCH:-claude/plugin-album-collection-5nvyi5}
SLUG=$1

if [ -z "$SLUG" ]; then
  echo "Which plugin? For example: sh install-plugin.sh bloodline-breaks-2"
  echo "Plugins with Mac builds: bloodline-breaks, bloodline-drums-2, bloodline-breaks-2"
  exit 1
fi
command -v gh >/dev/null || { echo "GitHub's command-line tool (gh) is not installed."; exit 1; }

# The workflow is named after the plugin; its download is the same name, capitalised, with -mac.
FIRST=$(printf %s "$SLUG" | cut -c1 | tr '[:lower:]' '[:upper:]')
ARTIFACT="$FIRST$(printf %s "$SLUG" | cut -c2-)-mac"

RUN=$(gh run list -R "$REPO" -w "$SLUG.yml" -b "$BRANCH" -s success -L 1 --json databaseId -q '.[0].databaseId')
if [ -z "$RUN" ]; then
  echo "No green $SLUG build on $BRANCH yet."
  exit 1
fi
echo "Downloading $ARTIFACT from run $RUN..."

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
gh run download "$RUN" -R "$REPO" -n "$ARTIFACT" -D "$TMP"
# The download holds one zip; unpack it beside itself.
for z in "$TMP"/*.zip; do [ -e "$z" ] && unzip -q "$z" -d "$TMP"; done

VST3=~/Library/Audio/Plug-Ins/VST3
AU=~/Library/Audio/Plug-Ins/Components
mkdir -p "$VST3" "$AU"
FOUND=0
for p in "$TMP"/*/*.vst3 "$TMP"/*/*.component; do
  [ -e "$p" ] || continue
  case "$p" in *.vst3) DEST=$VST3 ;; *) DEST=$AU ;; esac
  NAME=$(basename "$p")
  rm -rf "$DEST/$NAME"
  cp -R "$p" "$DEST/"
  xattr -dr com.apple.quarantine "$DEST/$NAME" 2>/dev/null || true
  echo "  $NAME"
  FOUND=1
done
[ "$FOUND" = 1 ] || { echo "That download has no plugins in it (some workflows only upload test renders)."; exit 1; }

# Makes macOS notice the new AU without a restart.
killall -9 AudioComponentRegistrar 2>/dev/null || true
echo "Installed. In Live: Preferences, Plug-Ins, Rescan."
