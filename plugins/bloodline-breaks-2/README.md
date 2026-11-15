# Bloodline Breaks 2 (JUCE prototype)

**Throwaway.** Bloodline Drums 2 and Bloodline Breaks in one instrument. An Audio Unit can't send MIDI to another track, so the loops play the drums inside the same plugin. This is the second pass ([SPEC.md](SPEC.md)): the user's notes from the first pass fixed. It answers: with those fixed, does breeding a kit and loops that play it, in one window, feel like one instrument?

```
 LEFT: drums                                   RIGHT: loops
 [ZONE None Kick Snare Hats Perc][back][new]   [STYLE None Boom bap Lo-fi Strung Neo soul][back][new]
 [LOOP Off Quarters Boom bap 16ths In break]   [Playing][Drag MIDI][swing][pull][spread]
   [SPREAD ----- 0.25]                         [A][B][C][D]
 [A][B][C][D]                                  settings: four lanes [sound | sound][One/Stack/Flam][Lock],
 settings: 19 settings, two columns              the free grid, this loop's swing
 [keep: 8 pads, 36 to 43]                      [keep: 8 loops, 48 to 55]
```

Saved sets from the first pass do not load (the state is now `bloodline-breaks-2 2`); the plugin starts fresh.

## Install on the Mac

1. **Quick way**, once GitHub's command-line tool is set up ([see the root README](../../README.md#installing-plugins-on-the-mac)). In Terminal, from any folder:
   ```
   gh api -H "Accept: application/vnd.github.raw" "repos/jimmyNicholas/audio-plugins/contents/tools/install-plugin.sh?ref=claude/plugin-album-collection-5nvyi5" | sh -s bloodline-breaks-2
   ```
   It installs the latest green build on the branch. **By hand** instead: from the [Bloodline Breaks 2 workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-breaks-2.yml), download **Bloodline-breaks-2-mac** from the latest green run, unzip it twice, and run `sh install-bloodline-breaks-2.sh` from inside the **Bloodline-breaks-2** folder.
2. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Breaks 2** (Plug-Ins, Jimmy Nicholas) on a MIDI track, VST3 or AU. It has its own plugin ID, so it sits beside the others.

## How to play it

**Left: build a kit.**
- **Zone:** None, Kick, Snare, Hats or Perc (toms, rims, woodblocks, clicks; a first guess at its ranges). None breeds anywhere.
- Click a tile (A to D) to hear it. **Breed**, **Cross**, **Back** and **New** work as in Bloodline Drums 2.
- **Spread** is a slider now: drag it to breed wider or narrower. Breeding still moves it (Again narrows, a new tile twice in a row widens), and **New** resets it.
- **Loop** plays the heard drum in a rhythm. **In break** plays it inside the right panel's loop, on its own lane (a snare candidate replaces the snare lane), and turns Playing on.
- **Keep:** drag a tile onto a pad. The pad is labelled with the zone ("Snare 3"; "Drum 2" from None, a wildcard any lane may use). **The litter stays as it is**, so you can keep the same tile or a sibling on another pad (two similar hats). Drag pads to swap them, back onto the litter to breed from them; **x** clears.

**Right: breed loops that play the kit.**
- **Style:** None (flat odds, no lean, no scatter, no rolls), Boom bap, Lo-fi, Strung (loose and drunk, kicks pushed about, snares late) or Neo soul (slow, swung, ghost snares). A soft pull, never a rule. Choosing a style moves the swing dial; None leaves it.
- **Playing is a switch, off to start.** Off, the right panel is silent whatever Live does. On, it follows Live's bar while Live plays, and runs at Live's tempo from the top of the bar while Live is stopped.
- **Drag MIDI** drags what is playing into a clip slot and **turns Playing off**, so the clip on the track plays the kit instead ("Playing off: the clip plays the kit" shows under the handle).
- **Swing**, **Pull** and **Spread** steer the next breed or New. Spread works as on the left.
- Four lanes, bottom up: kick, snare, hat, perc. **Every sound is a kept pad from the left; there are no built-in sounds.** A lane plays pads with its label first, then Drum pads, and now and then (more often for perc) a pad from another lane. With nothing kept, loops still breed but are silent.
- **Breeding always changes unlocked lanes:** each child makes at least one change you can see on every unlocked lane (a hit added, removed or moved more than half a sixteenth), and more as the spread grows.
- Click a loop tile to hear it. Its settings show below:
  - **The grid is a guide, not a cage.** Click empty space to add a hit on the nearest sixteenth. Drag a hit sideways to move it freely, up and down for its velocity (drawn as its height); hold **Shift** to snap to sixteenths; past either end it wraps round. Click a hit without moving to remove it. Swing bends time smoothly, so hits off the grid swing too.
  - **Each lane's head:** `[first sound | second sound][One/Stack/Flam][Lock]`. Click the first sound to cycle through the lane's choices (then none). **Drag a kept pad from the left onto either half** to set that sound. **+** adds a second sound (click to cycle), **x** clears it. The layer button cycles One, Stack (both at once) and Flam (the second follows 8 to 40 ms later, softer); in Flam it shows the ms, and dragging sideways on it changes them. With no second sound, Flam flams the first with itself.
  - **Lock** holds a lane when breeding; locked lanes get a faint lavender band here and on every tile.
- **Keep:** drag a loop onto a slot; the litter stays, so you can keep variations. Click a slot (or play 48 to 55) to switch to it; edits then change the kept loop. Drag slots to swap them or back onto the litter to breed from them.

**MIDI in:** 36 to 43 always play the pads, whatever Playing is. 48 to 55 switch to a kept loop. A dragged-out clip uses 36 to 43 only (a flam's second note at its delay and level), so it plays back through this plugin as it sounded.

**Saved with the set:** both litters, pads and their labels, kept loops, zone, locks, style, dial, pull, Loop and Playing. Back's history is not.

## Checklist

1. With the notes fixed, does breeding a kit and loops that play it, in one window, feel like one instrument?
2. **Free placement:** did dragging hits off the grid feel easy? Did Shift to snap get used? Is click to remove too easy to hit by accident?
3. **Flam:** did the bred flams sound right? Were 8 to 40 ms and 40 to 80% the right ranges? Did you set layers by hand by dragging pads?
4. **Keeping leaves the litter:** did keeping two similar hats work? Did you miss the fresh litter?
5. **Unlocking:** after unlocking a lane, did breeding visibly change it now? Too much at a low spread?
6. **Playing switch:** did Drag MIDI turning Playing off do what you wanted? Did In break turning it on?
7. **No built-ins:** did it matter that loops are silent until you keep a drum? Did the breeder borrow pads across lanes too often, or not enough?
8. **None and Perc:** are None style loops a useful blank? Does the Perc zone give toms, rims and clicks?
9. One thing to keep exactly, one to cut, one to add.

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the drum side (`drums`), the loops and their sound choices (`loops`), and the studio that ties them together, plus the player (`studio`). The litter comes from [`plugins/shared/breed/litter.h`](../shared/breed/litter.h); the voice comes straight from [`plugins/bloodline-drums/core`](../bloodline-drums/core).

```
cmake -S plugins/bloodline-breaks-2 -B build-b2 -G Ninja -DBLOODLINE_BREAKS2_PLUGIN=OFF   # core and tests; no JUCE
cmake --build build-b2 && ctest --test-dir build-b2 --output-on-failure
cmake -S plugins/bloodline-breaks-2 -B build -G Ninja -DCMAKE_BUILD_TYPE=Release          # everything, fetches JUCE 9.0.5
```

CI runs the tests and pluginval at strictness 10, and builds the universal Mac VST3 and AU ([workflow](../../.github/workflows/bloodline-breaks-2.yml)).

Known shortcuts: 4/4 and one bar only; no triplet grid; no taste learning; Back after swapping pads can bring back loops pointing at the old pad order; the style numbers and the Perc ranges are a first guess by ear; a flam's second note that falls past the bar's end wraps to the top in the dragged-out clip; Stack with no second sound does nothing until one is added; first-pass sets do not load.
