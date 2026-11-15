# Bloodline Synth 2 (JUCE prototype)

**Throwaway.** To Bloodline Synth what Bloodline Breaks 2 is to Bloodline Drums: sounds bred on the left, two, four or eight bar loops that play them bred on the right, in one instrument, so nothing has to send MIDI to another track (an Audio Unit cannot). It answers: does breeding loops, steered by a handful of musical traits, find synth parts worth keeping?

```
 LEFT: sounds                                  RIGHT: loops
 [zone][back][new]                             [bars][key][scale][back][new]
 [riff: Off Bass Chords Arp In loop][spread]   [playing][drag MIDI][octaves][spread]
 [A][B][C][D]                                  [A][B][C][D]
 [the heard sound's 24 settings]               [piano roll, lock strip, eight traits]
 [keep: 8 pads]                                [keep: 8 loops]
```

## Install on the Mac

1. **Quick way**, once GitHub's command-line tool is set up ([see the root README](../../README.md#installing-plugins-on-the-mac)). In Terminal, from any folder:
   ```
   gh api -H "Accept: application/vnd.github.raw" "repos/jimmyNicholas/audio-plugins/contents/tools/install-plugin.sh?ref=claude/plugin-album-collection-5nvyi5" | sh -s bloodline-synth-2
   ```
   It installs the latest green build on the branch. **By hand** instead: from the [Bloodline Synth 2 workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline-synth-2.yml), download **Bloodline-synth-2-mac** from the latest green run, unzip it twice, and run `sh install-bloodline-synth-2.sh` from inside the **Bloodline-synth-2** folder.
2. In Live: Preferences, Plug-Ins, Rescan. Load **Bloodline Synth 2** (Plug-Ins, Jimmy Nicholas) on a MIDI track, VST3 or AU. It has its own plugin ID (`Syn2`), so it sits beside Bloodline Synth.

## How to play it

**Left: breed and keep sounds.**
- **Zone** (None, Bass, Keys, Lead, Pad, Pluck) centres breeding, as in Bloodline Synth. None breeds anywhere.
- Click a tile (A to D) to hear it: a short C3, or the riff if one is on. **Breed**, **Cross**, **Back** and **New** work as in Bloodline Synth.
- **Riff** plays Bloodline Synth's Bass, Chords or Arp on the heard sound. **In loop** plays the right panel's loop with the heard sound in place of the loop's own, so you judge a candidate in the part. It turns Playing on.
- **Spread** is a slider: drag it to breed wider or closer. Breeding still narrows it (Again) and widens it (a new tile twice in a row); **New** puts it back to the start.
- **Keep:** drag a tile onto a pad. The pad is labelled with the zone ("Bass 2"; "Synth 3" for None). The litter stays as it is, so you can keep the same tile or a sibling again. Click a pad to hear it, drag pads to swap them, back onto the litter to breed from them; **x** clears.

**Right: breed loops that play what you kept.**
- **Bars:** 2, 4 or 8 for the litter. Fewer cuts each loop; more repeats it, and the next breed varies it.
- **Key** and **Scale** (Minor, Major, Dorian, Phrygian, Minor and Major pentatonic, Blues, Chromatic) cycle on click. They guide the Tonal trait and steer the next breed or New; no notes move.
- **Octaves:** drag either handle (C0 to C8, at least an octave apart). Notes outside are folded in on the next breed or New.
- **Play** is off by default, and off is silent whatever Live does. On: follows Live's position on the loop's own length (bar 1 of a four bar loop on every fourth bar) while Live plays, and runs at Live's tempo from the top when Live is stopped. **Drag MIDI** drags the loop, its full length, into a clip slot and turns Play off, so the clip plays the synth.
- Click a loop tile to play it. **Breed**, **Cross**, **Back**, **New** as on the left.
- **Keep:** drag a loop onto a slot; the litter stays. Click a slot to play it (edits then change the kept loop). Drag slots to swap them, back onto the litter to breed from them; **x** clears.

**Which sound plays a loop.** Every loop plays one kept sound. The breeder chooses it from the labels: a Bass for a low range (octaves centred below C3); Pluck or Lead for mono short notes, Lead for mono long ones, Pad or Keys for poly long ones, Keys or Pluck for poly short ones; then None (Synth) sounds; then any. It may choose again when breeding, and always when the sound was cleared. With nothing kept the loops breed but are silent. Click the **sound chip** above the roll to cycle through kept sounds, or drag a kept pad onto it.

**The loop's settings.**
- **Piano roll:** click empty space to add a note (a sixteenth long), drag a note to move it (freely in time, by semitone in pitch), drag its right edge to lengthen it, click it without moving to remove it. Shift snaps to sixteenths.
- **Lock strip:** drag along it to lock a stretch of time (snaps to sixteenths; Shift for free); click a locked stretch to unlock it. Locks belong to the litter: notes starting there pass down unchanged through Breed, Cross and trait moves, nothing new starts there, and notes running in are trimmed at its edge. They show as a shaded band on every tile.
- **Traits:** drag one, and on release the unlocked parts are drawn again from the new traits with the loop's own seed, so small moves give related results. **Lock** holds a trait through breeding.

### The traits

| Trait | 0 | 1 | What it steers |
| --- | --- | --- | --- |
| Tonal / Atonal | Tonal | Atonal | Pitches from the key and scale, rooted and resolving, at 0; chromatic and unrooted at 1 |
| Mono / Poly | Mono | Poly | Most notes at once: 1 at 0, up to 5 at 1; how often chords appear |
| Melodic / Rhythmic | Melodic | Rhythmic | At 0 the pitch moves in steps and leaps; at 1 a pitch or two repeat and the rhythm carries it |
| On beat / Off beat | On beat | Off beat | Onsets on beats and eighths at 0; off sixteenths and placement off the grid at 1 |
| Even / Dynamic | Even | Dynamic | Velocity: even at 0, wide with accents at 1 |
| Clustered / Open | Clustered | Open | Intervals in chords and between neighbours (seconds and thirds, up to fifths and octaves), and how much of the range is used |
| Sparse / Dense | Sparse | Dense | About 1 note a bar at 0, about 16 at 1 |
| Short / Long | Short | Long | Staccato at 0, legato and overlapping held notes at 1 |

A loop is drawn as a one bar motif (two bars for eight bar loops), repeated to fill, with about 30% of each repeat's notes moved, re-pitched, dropped or added so it develops. Breeding moves the traits, then makes at least one real change in every bar not locked, about `1 + spread * 8` a bar.

**MIDI in:** notes play the playing loop's sound, or the heard sound when nothing is kept or In loop is on, so a dragged-out clip plays back as it sounded. No note switches loops here; click a slot instead. No MIDI out.

**Saved with the set:** both litters, the pads and their labels, the kept loops, zone and setting locks, bars, key, scale, octaves, time locks, trait locks, riff (In loop included) and Play. Back's history is not.

## Checklist

1. Which traits did you use, and which did nothing you could hear?
2. Two, four or eight bars: which found keepers?
3. Did time locks let you keep the good half of a loop and breed the rest?
4. Did the breeder choose sensible sounds for the loops? Did you change them by hand often?
5. Did **In loop** help you choose sounds?
6. Moving a trait redraws the loop from its seed. Did small moves feel related, or like a new loop?
7. Did the piano roll do what you expected when you edited notes?
8. One thing to keep exactly, one to cut, one to add.

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the sound side (`sounds`), the loops, their traits, time locks, sound choice and the player (`loops`), and the studio that ties them together (`studio`). The litter comes from [`plugins/shared/breed/litter.h`](../shared/breed/litter.h); the patch, zones, engine and riff come straight from [`plugins/bloodline-synth/core`](../bloodline-synth/core).

```
cmake -S plugins/bloodline-synth-2 -B build-s2 -G Ninja -DBLOODLINE_SYNTH2_PLUGIN=OFF   # core and tests; no JUCE
cmake --build build-s2 && ctest --test-dir build-s2 --output-on-failure
cmake -S plugins/bloodline-synth-2 -B build -G Ninja -DCMAKE_BUILD_TYPE=Release        # everything, fetches JUCE 9.0.5
```

CI runs the tests and pluginval at strictness 10, and builds the universal Mac VST3 and AU, checked with `auval -v aumu Syn2 Jnic` ([workflow](../../.github/workflows/bloodline-synth-2.yml)).

Known shortcuts: 4/4 only, no swing; one sound a loop; a loop kept before any sound is kept gets one chosen when the first is kept; the parent stays put when breeding, so it keeps any notes outside a new octave range until it is bred from again; Back after swapping pads can bring back loops pointing at the old pad order; the trait mappings and the 30% variation are a first guess by ear.
