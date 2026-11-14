# Bloodline Breaks (JUCE prototype)

**Throwaway.** A prototype to answer one question: does breeding whole one-bar drum loops, the way Bloodline Synth breeds sounds, feel like a good way to find hip hop breaks? It borrows Bloodline Synth's interface (a litter of eight, Breed, Cross, Back, an inspector) and Bloodline Drums' voice for its built-in kit. Fold what works into the real plugin set; leave the rest here.

What it guesses:
- **A litter of eight one-bar loops**, of up to four drums on pads **36, 37, 38 and 39**. Click a tile and that bar loops; click another and the loop changes on its next hit, without stopping.
- **It starts random and learns.** A fresh litter is random placements, leaning on the style. Every Breed and Cross also teaches a **taste**: the chance of a hit on each step in what you keep choosing. Once it has learned something, fresh litters lean towards your taste too (up to halfway). Forget taste wipes it.
- **Everything is a soft pull, never a rule.** The **swing dial** (50% to 75%, MPC style: where the off sixteenth lands in its eighth) pulls each bar's own swing towards it; bars still differ. The **styles** (Boom bap, Lo-fi, Dilla, Trap, Drill, West coast, or Free) pull where hits land, how hard, how late, and whether hats roll. **Pull** sets how hard both pull: loose, soft, firm or tight.
- **Each lane has a role** (Kick, Snare, Hat, Perc, or Off for fewer than four drums). The role steers breeding (kicks like the one, snares the backbeat, hats the eighths) and picks the built-in sound.
- **Drag MIDI**: drag the heard bar straight into a clip slot in Live.

## Install on the Mac

1. Open the [Bloodline Breaks workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-breaks.yml), pick the latest green run on `claude/plugin-album-collection-5nvyi5`, and download **Bloodline-breaks-mac**.
2. Unzip it (a zip inside a zip; unzip both). In Terminal, from inside the **Bloodline-breaks** folder: `sh install-bloodline-breaks.sh`.
3. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Breaks** (Plug-Ins, Jimmy Nicholas) on a MIDI track. Use the **VST3** if you want its MIDI out (Live does not take MIDI out of an AU).

## How to play it

- **Hear:** click a tile (A to H). It loops: with Live playing, on Live's bar; with Live stopped, press **Play** and it loops at Live's tempo.
- **Breed:** the tile's Breed button. It stays as the parent (starred); the other seven are bred from it. Breeding from the parent again narrows the spread; from a new tile twice in a row widens it.
- **Cross:** mates the tile with the one you are hearing; each lane comes whole from one parent or the other, so you can get A's kick and snare under C's hats.
- **Back** undoes the last breed, cross or fresh litter (not the taste). **Fresh** draws eight new bars.
- **Style** moves the swing dial to that style's usual swing; move the dial after if you like. Style, dial and Pull only steer the next breed or fresh litter.
- **Inspector:** the heard bar, big. Click a step: hit, ghost, rest. Click a lane's role to change it (Off empties that lane everywhere). **Lock** holds a lane while breeding. Drag **This bar's swing** to change just this bar.
- **Kit on** plays the built-in sounds as well as sending MIDI. **MIDI only** silences them, for when you route this track's MIDI to Bloodline Drums or a Drum Rack (on the other track: MIDI From, this track, Bloodline Breaks).
- **Saved with the set:** the litter, which bar you hear, the parent, spread, style, dial, pull, roles, locks, taste, Play and Kit. Back's history is not.

Blocks in the grids sit where the hit really lands, after swing and nudge; a split block is a roll.

## Checklist

1. Does clicking through a litter while it loops feel like the drum loop did in the first Bloodline spike?
2. How many bars until something is worth keeping? With which style?
3. Is the swing dial soft enough, or too soft? Did you want it to be a hard MPC swing after all? Same for the styles. Try Pull at loose and at tight.
4. Does the taste help? After ten or so breeds, do fresh litters feel more like you?
5. Are the styles right? Which is furthest off, and what would you change?
6. Four lanes on 36 to 39: enough? Did you want more steps (two bars, triplets) or fewer?
7. Kit on or MIDI only: which did you use? Did routing to Bloodline Drums work?
8. Did Drag MIDI land in Live as you expected?
9. One thing to keep exactly, one to cut, one to add.

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the pattern, styles and taste (`pattern`), the breeder and its saved text (`breeder`), and the loop player (`player`). The litter comes from [`plugins/shared/breed/litter.h`](../shared/breed/litter.h); the built-in kit uses Bloodline Drums' voice straight from [`plugins/bloodline-drums/core`](../bloodline-drums/core), not moved into `shared/`, since this is a prototype.

```
cmake -S plugins/bloodline-breaks -B build-breaks -G Ninja -DBLOODLINE_BREAKS_PLUGIN=OFF   # core and tests; no JUCE
cmake --build build-breaks && ctest --test-dir build-breaks --output-on-failure
cmake -S plugins/bloodline-breaks -B build -G Ninja -DCMAKE_BUILD_TYPE=Release            # everything, fetches JUCE 9.0.5
```

CI runs the tests and pluginval at strictness 10, and builds the universal Mac VST3 and AU ([workflow](../../.github/workflows/bloodline-breaks.yml)).

Known shortcuts: 4/4 only; one bar only; notes are 30 ms long; the style numbers are a first guess by ear.
