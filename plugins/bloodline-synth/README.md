# Bloodline Synth (JUCE spike)

A guess at what a synth version of the breeders could look like, built alongside the second Bloodline spike ([#47](https://github.com/jimmyNicholas/audio-plugins/issues/47)). Synths belong to a later album, not Bloodline Breaks (see the map, [#29](https://github.com/jimmyNicholas/audio-plugins/issues/29)), so this is a sketch to react to, not a decision. It is named Bloodline Synth so the three spikes sit together in Live's plug-in list. It is mouse led: you breed by clicking, and play with your keyboard, the on-screen keys, or a built-in riff.

What it guesses:
- **A litter of eight**, not three pads. A synth sound is harder to judge in one hit than a drum, so you hear more siblings side by side.
- **Held notes follow the patch.** Click through the litter while a chord or the riff plays and the sound changes under your hands, the way playing all three sibling drums in one loop was the best part of the first Bloodline spike.
- **Cross**: mate two candidates. Whole groups (oscillators, filter, envelope, motion, out) pass down from one parent or the other, sometimes blended, so a child sounds like one parent's filter on the other's oscillators.
- **Back**, so a wild jump is never a loss.
- **Zones** as in Bloodline: Bass, Keys, Lead, Pad and Pluck centre breeding; None breeds anywhere.
- **Ranges well outside the norms**: FM, a sub, noise, resonance near self-oscillation, filter envelope up to six octaves, wobble up to four octaves, down to 3 bits.

## The sound

Eight voices. Two oscillators that morph from sine to triangle, saw and square; B tuned up to two octaves either way and phase-modulating A (FM); a sub an octave down; noise. Then drive and a resonant low-pass with its own envelope, an LFO for vibrato and wobble, glide, and width (each side detuned a little apart). 24 settings in five groups:

| Group | Settings |
| --- | --- |
| Osc | Shape A, Shape B, Tune B, Detune, Mix B, Sub, Noise, FM |
| Filter | Cutoff, Resonance, Env amount, Filter decay, Drive |
| Amp | Attack, Decay, Sustain, Release |
| Motion | LFO rate, Vibrato, Wobble, Glide |
| Out | Bits, Width, Level |

Level is never bred, as in Bloodline. A soft knee keeps stacked voices from clipping hard.

## Install on the Mac

1. Open the [Bloodline Synth workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-synth.yml), pick the latest green run on `claude/plugin-album-collection-5nvyi5`, and download **Bloodline-synth-mac**.
2. Unzip it (a zip inside a zip; unzip both). You get a **Bloodline-synth** folder of its own. In Terminal, from inside that folder: `sh install-bloodline-synth.sh`. It copies the VST3 and AU into `~/Library/Audio/Plug-Ins`, clears the quarantine flag (the plugin is not signed), and removes the copy installed under the old name, **Synth Breeder**, which has the same plugin ID.
3. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Synth** (Plug-Ins, Jimmy Nicholas) on a MIDI track, VST3 or AU, not both.

## How to play it

- **Hear:** click a tile (A to H). It becomes the sound you play, at once, even under held notes.
- **Play:** your MIDI keyboard, the on-screen keys, or **Riff** (Bass, Chords or Arp, at Live's tempo) to keep something playing while you click.
- **Breed:** the tile's Breed button. It stays as the parent (starred); the other seven are bred from it. Breeding from the parent again narrows the spread; from a new tile twice in a row widens it.
- **Cross:** the Cross button on any tile mates it with the one you are hearing. Both parents stay; the other six are their children.
- **Back** undoes the last breed, cross or fresh litter. **Fresh litter** draws eight new sounds.
- **Inspector** (right): every setting of the tile you are hearing. Drag to change it; **Lock** keeps a setting when breeding.
- **Saved with the set:** the litter, which tile you hear, the parent, spread, zone, locks and the riff. Back's history is not.

## Checklist

Post notes on [#47](https://github.com/jimmyNicholas/audio-plugins/issues/47), or wherever the synth album is mapped later.

1. Does clicking through a litter while the riff plays feel like the drum loop did in Bloodline?
2. Eight tiles: too many, too few, or right?
3. Is Cross worth having? Do children sound like their parents?
4. Do the zones start you in the right places? How often is a fresh tile usable?
5. Which settings would you cut, and what is missing?
6. Mouse led, or did you want the MPD226 or keys to drive breeding here too?

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the patch and zones, the engine, the riff, and the breeder. The breeding loop itself (a litter, breed, cross, back, spread) is shared with Bloodline Effects in [`plugins/shared/breed/litter.h`](../shared/breed/litter.h).

```
cmake -S plugins/bloodline-synth -B build-synth -G Ninja -DBLOODLINE_SYNTH_PLUGIN=OFF   # core, tests, render; no JUCE
cmake --build build-synth && ctest --test-dir build-synth --output-on-failure
build-synth/synth_render <out dir> [seed]                                        # litter.wav, breed.wav
cmake -S plugins/bloodline-synth -B build -G Ninja -DCMAKE_BUILD_TYPE=Release       # everything, fetches JUCE 9.0.5
```

Tests: `tests/litter_tests.cpp` (the shared litter, with patches) and `tests/engine_tests.cpp` (the sound, the riff, saving). CI runs pluginval at strictness 10 and builds the universal Mac VST3 and AU ([workflow](../../.github/workflows/bloodline-synth.yml)).

Known shortcuts, fine for a spike: FM and the triangle alias; one LFO per voice, restarted with each new voice; the audio thread and the editor share one lock.
