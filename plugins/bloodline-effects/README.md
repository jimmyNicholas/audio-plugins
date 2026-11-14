# Bloodline Effects (JUCE spike)

A first go at the effects version you asked for after the first Bloodline spike: reverb, delay, compression, saturation, filter, widening and more, on one drum or the whole bus, with the chain itself bred. Built alongside the second Bloodline spike ([#47](https://github.com/jimmyNicholas/audio-plugins/issues/47)). Whether it belongs in Bloodline Breaks is still [#46](https://github.com/jimmyNicholas/audio-plugins/issues/46); this is something to play before deciding. It is mouse led and experimental with its choices.

What it guesses:
- **The chain is the sound.** Each candidate is up to six effects in order, each with four settings, plus a dry/wet mix. Breeding changes the settings, and also the chain: an effect can turn into another, swap with its neighbour, appear or vanish.
- **Wild** makes those jumps three times as likely, and sometimes shuffles the whole order. Off, the chain mostly keeps its shape and the settings drift.
- **Characters** centre which effects appear, as zones do in Bloodline: Space (reverb, delay, width, comb), Dirt (drive, crush, tape, compressor), Motion (filter, comb, gate, width), Glitch (glitch, ring, gate, crush). None draws from all twelve.
- **A litter of six**, Cross (the front of one chain and the back of the other), and Back.
- **Safe to be wild:** a DC blocker and a limiter at -1 dB sit after every chain and are never bred. The header shows when the limiter is working.
- **On a pad or on the bus:** it is an audio effect, so put it on one pad's chain inside a Drum Rack, or on the drum group. **Loop is input** feeds it a built-in boom bap loop at Live's tempo, to audition without a clip.

## The effects

| Effect | Settings |
| --- | --- |
| Drive | Amount (to x60), Fold (tanh to wavefolding), Tone, Bias (asymmetry) |
| Crush | Bits (16 down to 1.5), Rate (48 kHz down to 200 Hz), Jitter, Mix |
| Filter | Cutoff, Resonance (near self-oscillation), Mode (low, band, high), Sweep (an LFO up to 4 octaves) |
| Comb | Time (0.2 to 25 ms), Feedback (either sign), Depth, Rate: flanger to metallic resonator |
| Delay | Time (10 ms to 1.5 s), Feedback (past 100%, held back by saturation), Tone, Ping-pong |
| Reverb | Size, Decay (the top freezes the tail), Damping, Mix |
| Gate | Rate, Duty, Depth, Smooth: chops and tremolo |
| Ring | Freq (5 Hz to 5 kHz), Shape, Drift, Mix |
| Tape | Wow, Wow rate, Flutter, Saturate |
| Comp | Threshold, Ratio (to 30:1), Attack, Release, with makeup gain |
| Width | Width (mono to double), Haas delay, Spin (auto-pan) rate and depth |
| Glitch | Chance, Length, Speed (backwards below zero), Mix: grabs a slice and repeats it |

## Install on the Mac

1. Open the [Bloodline Effects workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-effects.yml), pick the latest green run on `claude/plugin-album-collection-5nvyi5`, and download **Bloodline-effects-mac**.
2. Unzip it (a zip inside a zip; unzip both). You get a **Bloodline-effects** folder of its own. In Terminal, from inside that folder: `sh install-bloodline-effects.sh`. It copies the VST3 and AU into `~/Library/Audio/Plug-Ins`, clears the quarantine flag (the plugin is not signed), and removes the copy installed under the old name, **FX Breeder**, which has the same plugin ID.
3. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Effects** (Plug-Ins, Jimmy Nicholas) as an audio effect, VST3 or AU, not both.

## How to play it

- **Hear:** click a card (A to F). The chain fades in over 40 ms on whatever is playing.
- **Breed:** the card's Breed button. It stays as the parent; the other five are bred from it. Breeding from the parent again narrows the spread (fewer structural jumps); from a new card twice in a row widens it.
- **Cross with hearing:** mates that card with the one you hear.
- **Back**, **Fresh litter**, **Wild**, **Character**: above the cards.
- **The heard chain** (bottom): each effect with its four settings. Drag to change one. **Lock** keeps an effect exactly as it is when breeding (never changed, swapped or removed). **x** removes it. **Mix** is the chain's dry/wet.
- **Bypass** fades to the untouched input. **Out** sets the output level, before the limiter.
- **Saved with the set:** the litter, which card you hear, the parent, spread, character, Wild, the input, bypass and output. Back's history is not.

## Checklist

Post notes on [#46](https://github.com/jimmyNicholas/audio-plugins/issues/46), since that decides whether this is part of Bloodline Breaks.

1. On a single pad, then on the drum bus: which is more useful?
2. Does breeding the chain itself (not just settings) find things you would not have built by hand?
3. Wild: exciting, or just random? Is the calm setting too calm?
4. Which effects earn their place? Which would you cut, and what is missing (pitch, reverse, sidechain)?
5. Did the limiter ever squash something you wanted loud?
6. Mouse led, or do you want pads (Bank B?) to drive it?
7. One plugin with the drums, or its own plugin, or its own album?

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the chain and how it breeds, the twelve effects, the rack and player (fades, bypass, DC blocker, limiter), the built-in loop and the breeder. The breeding loop is shared with Bloodline Synth in [`plugins/shared/breed/litter.h`](../shared/breed/litter.h).

```
cmake -S plugins/bloodline-effects -B build-effects -G Ninja -DBLOODLINE_EFFECTS_PLUGIN=OFF   # core, tests, render; no JUCE
cmake --build build-effects && ctest --test-dir build-effects --output-on-failure
build-effects/effects_render <out dir> [seed]                                     # dry.wav, litter.wav, breed.wav, and what each chain is
cmake -S plugins/bloodline-effects -B build -G Ninja -DCMAKE_BUILD_TYPE=Release  # everything, fetches JUCE 9.0.5
```

Tests: `tests/chain_tests.cpp` (drawing, breeding, Wild, locks, crossing, saving) and `tests/rack_tests.cpp` (every effect bounded at its extremes, mix, bypass, switching racks). CI runs pluginval at strictness 10 and builds the universal Mac VST3 and AU ([workflow](../../.github/workflows/bloodline-effects.yml)).

Known shortcuts, fine for a spike: a new rack is built whenever the chain's effects or order change, so reverb and delay tails restart then (only settings changes keep them); the reverb is a plain Schroeder; Crush and Ring alias on purpose; the audio thread and the editor share one lock.
