# Bloodline Drums 2 (JUCE prototype)

**Throwaway.** A second take on Bloodline Drums, built like Bloodline Synth: a litter of eight drum sounds to click through and breed, with **eight keep pads where the synth has its keyboard**. Drag a sound you like onto a pad to keep it, and the litter starts fresh so the next drum can be bred. It answers: is breeding in a litter and keeping onto pads a better way to build a kit than Bloodline Drums' workbench and slots?

It breeds Bloodline Drums' 19-setting voice as it is, with the same zones (None, Kick, Snare, Hats), locks and spread. Cross is Bloodline Synth's: whole groups (Body, Noise, Click, Grit, Out) come from one parent or the other.

```
 [A] [B] [C] [D]      inspector:
 [E] [F] [G] [H]      every setting of the
                      sound you are hearing,
 KEEP                 drag to turn, Lock
 [36][37][38][39][40][41][42][43]
```

## Install on the Mac

1. **Quick way**, once GitHub's command-line tool is set up ([see the root README](../../README.md#installing-plugins-on-the-mac)). In Terminal, from any folder:
   ```
   gh api -H "Accept: application/vnd.github.raw" "repos/jimmyNicholas/audio-plugins/contents/tools/install-plugin.sh?ref=claude/plugin-album-collection-5nvyi5" | sh -s bloodline-drums-2
   ```
   It installs the latest green build on the branch. **By hand** instead: from the [Bloodline Drums 2 workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-drums-2.yml), download **Bloodline-drums-2-mac** from the latest green run, unzip it twice, and run `sh install-bloodline-drums-2.sh` from inside the **Bloodline-drums-2** folder.
2. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Drums 2** (Plug-Ins, Jimmy Nicholas) on a MIDI track, VST3 or AU, not both. It has its own plugin ID, so it sits beside Bloodline Drums.

## How to play it

- **Hear:** click a tile (A to H), or hit 44 to 51 on the MPD226 (Bank A, top two rows). **Loop** plays the sound you are hearing in a rhythm (Quarters, Boom bap or 16ths) at Live's tempo, so you can click through while it plays.
- **Breed**, **Cross**, **Back** and **Fresh litter** work as in Bloodline Synth. **Zone** centres breeding on Kick, Snare or Hats; None breeds anywhere.
- **Keep:** drag a tile onto a pad. The pad keeps that sound (named after the zone, such as "Kick 3") and the litter starts fresh with the same zone and locks. Back brings the old litter back; the pad stays kept. Dropping on a full pad replaces it.
- **Play the kit:** click a pad, hit 36 to 43 (Bank A, bottom two rows), or put a clip on those notes.
- **Rearrange:** drag a pad onto another to swap them. **x** clears a pad.
- **Breed from a pad:** drag it back up onto the litter. Its sound becomes the parent of a new litter.
- **Saved with the set:** the litter, the eight pads and their names, zone, locks and loop. Back's history is not.

## Checklist

1. Breed, keep, breed, keep: does building a kit this way feel better or worse than Bloodline Drums' workbench and slots?
2. Is a fresh litter the right thing after keeping, or did you want to carry on from where you were?
3. Eight tiles for drums: too many to judge, or useful?
4. Did you use Loop, or the MPD226's top rows, to audition?
5. Did you drag pads back onto the litter to breed them again? Did you want to turn a kept pad's settings by hand?
6. Is Cross useful for drums? Do the children sound like both parents?
7. One thing to keep exactly, one to cut, one to add.

## Building and testing here

The core (`core/kit`) is plain C++17 with no JUCE: the breeding rules for the litter, the pads, the loop and the saved text. The litter comes from [`plugins/shared/breed/litter.h`](../shared/breed/litter.h); the voice and settings come straight from [`plugins/bloodline-drums/core`](../bloodline-drums/core), not moved into `shared/`, since this is a prototype.

```
cmake -S plugins/bloodline-drums-2 -B build-d2 -G Ninja -DBLOODLINE_DRUMS2_PLUGIN=OFF   # core and tests; no JUCE
cmake --build build-d2 && ctest --test-dir build-d2 --output-on-failure
cmake -S plugins/bloodline-drums-2 -B build -G Ninja -DCMAKE_BUILD_TYPE=Release         # everything, fetches JUCE 9.0.5
```

CI runs the tests and pluginval at strictness 10, and builds the universal Mac VST3 and AU ([workflow](../../.github/workflows/bloodline-drums-2.yml)).
