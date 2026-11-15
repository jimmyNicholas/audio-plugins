# Bloodline Breaks 2 (JUCE prototype)

**Throwaway.** Bloodline Drums 2 and Bloodline Breaks in one instrument. An Audio Unit can't send MIDI to another track, so the loops now play the drums inside the same plugin. It answers: is breeding drums on the left and loops that play them on the right a good way to build a beat from nothing?

```
 LEFT: drums                         RIGHT: loops
 [zone][back][new]                   [style][back][new]
 [loop][spread]                      [playing][midi][swing][pull]
 [A][B][C][D]                        [A][B][C][D]
 [selected settings]                 [selected settings]
 [keep: 8 pads, 36 to 43]            [keep: 8 loops, 48 to 55]
```

## Install on the Mac

1. Open the [Bloodline Breaks 2 workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-breaks-2.yml), pick the latest green run on `claude/plugin-album-collection-5nvyi5`, and download **Bloodline-breaks-2-mac**.
2. Unzip it (a zip inside a zip; unzip both). In Terminal, from inside the **Bloodline-breaks-2** folder: `sh install-bloodline-breaks-2.sh`.
3. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Breaks 2** (Plug-Ins, Jimmy Nicholas) on a MIDI track, VST3 or AU. It has its own plugin ID, so it sits beside the others.

## How to play it

**Left: build a kit.**
- **Zone** (Kick, Snare, Hats, Other) centres breeding. Other breeds anywhere.
- Click a tile (A to D) to hear it. **Breed**, **Cross**, **Back** and **New** work as in Bloodline Drums 2.
- **Loop** plays the heard drum in a rhythm. **In break** is new: it plays the heard drum inside the right panel's loop, on its own lane (a snare candidate replaces the snare lane), so you hear it in the beat. It starts the loop if nothing else is.
- **Keep:** drag a tile onto a pad. The pad is labelled with the zone (such as "Snare 3") and the litter starts fresh. Drag pads to swap them, back onto the litter to breed from them; **x** clears.

**Right: breed loops that play the kit.**
- **Style:** Boom bap, Lo-fi, Strung (loose and drunk, kicks pushed about, snares late) or Neo soul (slow, swung, ghost snares). A soft pull, never a rule. Choosing one moves the swing dial.
- **Play** loops at Live's tempo when Live is stopped; when Live plays, it follows Live's bar. **Drag MIDI** drags what is playing into a clip slot. **Swing** and **Pull** steer the next breed or New.
- Four lanes, bottom up: kick, snare, hat, perc. Each lane plays kept pads with its label; perc plays Other pads, or any kept drum if there are none. **Which pad, and whether to layer two, is the breeder's choice**, and it changes as you breed. A lane with no kept pad to play uses a built-in sound.
- Click a loop tile to play it. Its settings show below: click a step (hit, ghost, rest), click a lane's sound to choose another (one pad at a time, then built-in), **Lock** holds a lane when breeding.
- **Keep:** drag a loop onto a slot. Unlike the drums, the litter stays, so you can keep variations. Click a slot (or play 48 to 55) to switch to it; edits then change the kept loop. Drag slots to swap them or back onto the litter to breed from them.

**MIDI in:** 36 to 43 play the pads, 44 to 47 the built-in kick, snare, hat and perc, 48 to 55 switch to a kept loop. A dragged-out clip uses the same notes, so it plays back through this plugin as it sounded.

**Saved with the set:** both litters, pads and their labels, kept loops, zone, locks, style, dial, pull, Loop and Play. Back's history is not.

## Checklist

1. Does breeding a kit then breeding loops on it, in one window, feel like one instrument or two stuck together?
2. Did **In break** help you choose drums? Did you want it on by default?
3. Four tiles a side: enough, or did you miss the eight?
4. The breeder picks which kick and whether to layer snares. Did its choices sound right? Too much layering, too little? Did you change sounds by hand often?
5. Are the four styles right? Which is furthest off? Is Strung strung enough?
6. Keeping loops without a fresh litter: right, or should it start fresh like the drums?
7. Did switching kept loops from 48 to 55 (or a clip) work as a way to arrange?
8. One thing to keep exactly, one to cut, one to add.

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the drum side (`drums`), the loops and their sound choices (`loops`), and the studio that ties them together, plus the player (`studio`). The litter comes from [`plugins/shared/breed/litter.h`](../shared/breed/litter.h); the voice comes straight from [`plugins/bloodline-drums/core`](../bloodline-drums/core).

```
cmake -S plugins/bloodline-breaks-2 -B build-b2 -G Ninja -DBLOODLINE_BREAKS2_PLUGIN=OFF   # core and tests; no JUCE
cmake --build build-b2 && ctest --test-dir build-b2 --output-on-failure
cmake -S plugins/bloodline-breaks-2 -B build -G Ninja -DCMAKE_BUILD_TYPE=Release          # everything, fetches JUCE 9.0.5
```

CI runs the tests and pluginval at strictness 10, and builds the universal Mac VST3 and AU ([workflow](../../.github/workflows/bloodline-breaks-2.yml)).

Known shortcuts: 4/4 and one bar only; no taste learning (Bloodline Breaks' taste had no room); Back after swapping pads can bring back loops pointing at the old pad order; the style numbers are a first guess by ear.
