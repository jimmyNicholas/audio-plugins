# Bloodline Synth 2 (spec)

**For:** a fresh session, building a new throwaway prototype in this folder. It is to Bloodline Synth what Bloodline Breaks 2 is to Bloodline Drums: **sounds bred on the left, loops that play them bred on the right, in one instrument**, so nothing has to send MIDI to another track (an Audio Unit cannot).

Read first:
- [Bloodline Synth's README](../bloodline-synth/README.md): the sound (24 settings in five groups, zones, the riff), which this prototype reuses as it is.
- [Bloodline Breaks 2](../bloodline-breaks-2/): the two-panel layout, the keep rows, carrying tiles about, and the core/plugin split to copy. Read its [SPEC.md](../bloodline-breaks-2/SPEC.md) too: build its fixes (None options, draggable spread, keeping leaves the litter, explicit Playing, free placement, no built-in sounds) into this one from the start.

**The question it answers:** does breeding two, four or eight bar loops, steered by a handful of musical traits, find synth parts worth keeping?

## Shape

- New plugin: `plugins/bloodline-synth-2`, product **Bloodline Synth 2**, manufacturer `Jnic`, plugin code `Syn2`, VST3, AU and Standalone, an instrument with MIDI in and no MIDI out. Its own CI workflow, install script and README, copied from Bloodline Breaks 2's and renamed. Add a row to the root README's plugin table.
- Core in `core/`, plain C++17, no JUCE, tested with `plugins/shared/test/check.h`. The patch, zones, engine and riff come straight from `plugins/bloodline-synth/core` (add its `patch.cpp`, `engine.cpp` and `riff.cpp` to this CMake target, as Breaks 2 does with the drum voice). Do not move or change them.
- The litter is `plugins/shared/breed/litter.h`, unchanged.
- Window about 1280 by 780. Two panels of 616 px.

## Left panel: sounds

```
[ZONE None Bass Keys Lead Pad Pluck][Back][New]
[RIFF Off Bass Chords Arp In loop][SPREAD ----- 0.25]
[A][B][C][D]
[selected settings]
[KEEP: 8 pads]
```

- **Zone:** Bloodline Synth's zones, None first. None breeds anywhere.
- **Riff:** Bloodline Synth's riff (Bass, Chords, Arp) playing the heard sound, or Off. **In loop** plays the right panel's loop with the heard sound in place of the loop's own, so a candidate is judged in the part. Choosing it turns Playing on.
- **Spread:** a slider (as in Breaks 2). New resets it.
- **A to D:** a litter of four. Click to hear (a short C3 note, or the riff if on). Breed, Cross, Back, New as in Bloodline Synth.
- **Selected settings:** all 24 settings of the heard sound in two columns (Osc and Filter; Amp, Motion and Out), drag to turn, Lock each. Level is never bred.
- **Keep:** drag a tile onto one of eight pads. The pad is labelled with the zone ("Bass 2"; "Synth 3" for None). **The litter stays as it is.** Click a pad to hear it. Drag pads to swap, back onto the litter to breed from them, x to clear.

## Right panel: loops

```
[BARS 2 4 8][KEY C][SCALE Minor][Back][New]
[Playing][Drag MIDI][OCTAVES o====o C2 to C5][SPREAD ---]
[A][B][C][D]
[selected settings: piano roll, lock strip, traits]
[KEEP: 8 loops]
```

- **Bars:** 2, 4 or 8, for the litter. Changing it: shorter loops are cut; longer ones repeat what is there to fill, and the next breed varies it.
- **Key** (C to B) and **Scale** (Minor, Major, Dorian, Phrygian, Minor pentatonic, Major pentatonic, Blues, Chromatic) cycle on click. They guide the Tonal trait. Changing them moves no notes; it steers the next breed or New.
- **Octaves:** a dual slider with two handles, the lowest and highest octave notes may use (C0 to C8, default C2 to C5, at least one octave apart). Breeding keeps notes inside; notes outside after a change are folded in by octaves on the next breed or New.
- **Playing:** off by default; off is silent whatever Live does. On: follows Live's bar while Live plays (on the loop's own length: bar 1 of the loop on every 2, 4 or 8 bar boundary); runs at Live's tempo when Live is stopped. **Drag MIDI** drags what is playing into a clip slot and turns Playing off, so the clip takes over.
- **Spread:** as on the left.
- **A to D:** four loops, each drawn as a small piano roll of the whole loop. Click to play it. Breed, Cross.
- **Keep:** drag a loop onto one of eight slots; the litter stays. Click a slot to play it (edits then change the kept loop). Swap, breed from, clear, as in Breaks 2.

### Which sound plays a loop

Every loop plays **one kept sound from the left panel**. There are no built-in sounds; with nothing kept, loops breed but are silent, with the note "Keep a sound on the left to hear the loops."

The breeder chooses the sound, guided by the labels:
- Low range (octave centre below C3): Bass first.
- Mono, short notes: Pluck or Lead. Mono, long: Lead.
- Poly, long: Pad or Keys. Poly, short: Keys or Pluck.
- Then None-labelled sounds, then any kept sound.

Mutation re-chooses with chance `spread * 1.5` (at most 0.6), and always when the sound has been cleared. By hand: the sound chip at the top of the settings cycles through kept sounds, and a kept pad dragged onto it sets it.

### The traits

Each loop has eight traits, each a value from 0 to 1 between two poles. They are part of the loop: bred, crossed, shown and editable per loop. The notes are drawn from them.

| Trait | 0 | 1 | What it steers |
| --- | --- | --- | --- |
| Tonal / Atonal | Tonal | Atonal | Pitches from the key and scale, rooted and resolving, at 0; chromatic and unrooted at 1 |
| Mono / Poly | Mono | Poly | Most notes at once: 1 at 0, up to 5 at 1; how often chords appear |
| Melodic / Rhythmic | Melodic | Rhythmic | At 0, pitch moves (a contour of steps and leaps); at 1, few pitches repeat and the rhythm carries it |
| On beat / Off beat | On beat | Off beat | Onsets on beats and eighths at 0; off sixteenths, syncopation and placement off the grid at 1 |
| Dynamic low / high | Low | High | Velocity spread: even at 0, wide with accents at 1 |
| Clustered / Open | Clustered | Open | Intervals within chords and between neighbouring notes: seconds and thirds at 0, fifths and octaves and more at 1; how much of the octave range is used |
| Sparse / Dense | Sparse | Dense | Notes a bar: about 1 at 0, about 16 at 1 |
| Short / Long | Short | Long | Note length: staccato at 0, legato and overlapping held notes at 1 |

(Sparse / Dense and Short / Long are the two added to the user's list: they shape a synth part as much as the others, and nothing else covers them.)

**Breeding the traits:** fresh draws each uniformly; mutation moves each by `gauss * spread`, folded into 0 to 1; cross takes each from one parent or the other, sometimes blended. A trait's **Lock** holds it through breeding.

**By hand:** drag a trait's slider in the settings. On release, the playing loop's unlocked parts (see time locks) are drawn again from the new traits, using the loop's own seed, so small moves give related results rather than a new loop. While dragging, only the slider moves.

### Notes and drawing them

**Data:** a loop is its bars, sound, eight traits, a seed, and a list of notes (at most 256). A note has `start` (in beats, a double, anywhere in the loop), `length` (beats, at least 1/64 of a bar), `pitch` (MIDI note) and `velocity` (0.08 to 1).

**Drawing a loop from its traits** (fresh, or a trait changed by hand):
1. Draw a motif one bar long (two bars for an eight bar loop): onsets from Sparse / Dense and On beat / Off beat, lengths from Short / Long, velocities from Dynamic, pitches and chords from Tonal, Mono / Poly, Melodic / Rhythmic and Clustered / Open, inside the octave range.
2. Fill the loop by repeating the motif, varying each repeat a little (about 30% of its notes moved, re-pitched, dropped or added, a constant to tune by ear), so longer loops develop rather than copy.

**Mutation:** traits move first; then notes change, guided by the new traits: at least one real change per bar not locked, and about `1 + spread * 8` changes a bar on average, chosen from add, remove, move in time (`gauss * spread` beats), re-pitch (by scale step when tonal), and change length or velocity. Children must differ audibly from the parent at every spread (Breaks 2 found unlocked lanes looked locked when mutation was too gentle; test it).

**Cross:** each bar from one parent or the other, traits mixed as above.

### Time locks

A **lock strip** under the piano roll in the settings, the loop's full length.
- Drag along it to lock a range (snaps to sixteenths; Shift for free). Several ranges may be locked. Click a range to unlock it.
- Locked ranges belong to the litter, not one loop, like the drum lane locks: they apply to every candidate when breeding, and show as a shaded band on every tile and in the roll.
- When breeding, crossing or re-drawing from traits: notes starting in a locked range are copied exactly; nothing new starts there; a note from an unlocked part that would run into a locked range is trimmed to its edge.
- Ranges past the loop's end (after switching to fewer bars) are kept but have no effect.

### Selected settings (the right panel's settings)

```
PLAYING B ★       [sound chip: Bass 2]
[piano roll: pitch up, time across, bar lines, playhead]
[lock strip]
Tonal ----o---- Atonal    [Lock]     Sparse ---o----- Dense   [Lock]
Mono  --o------ Poly      [Lock]     Short  ------o-- Long    [Lock]
...four rows, two columns
```

**Editing the roll** (as the drum grid in Breaks 2): click empty space to add a note at the nearest sixteenth (length a sixteenth, velocity 0.8); press and drag a note to move it, freely in time and by semitone in pitch; drag a note's right edge to change its length; Shift snaps to sixteenths; click without moving removes it. Vertical scale: the octave range, with key notes shaded when tonal.

## MIDI

- **In:** notes play the playing loop's sound (or the heard left candidate if nothing is kept), so a dragged-out clip plays back as it sounded. With In loop on, they play the heard candidate.
- **Out:** none. The dragged-out MIDI file holds the loop's notes as they are, its full length.
- No note switches kept loops here (every note may be melodic); switching is by clicking a slot.

## Voices

One Bloodline Synth `Engine` for each kept pad, and one for the heard candidate. A loop's notes go to its sound's engine. Each engine has its eight voices, as now.

## Saved with the set

Both litters, the eight sound pads and their labels, the eight kept loops, zones and setting locks, bars, key, scale, octaves, time locks, trait locks, riff, In loop and Playing. Back's history is not. Text, line based, starting `bloodline-synth-2 1`, as Breaks 2 does.

## Tests (core, no JUCE)

1. Traits steer: across many seeds, Dense loops have more notes than Sparse; Mono loops never sound two notes at once; Tonal loops (0) use only the scale's notes; Long loops have longer notes than Short; Open has wider intervals than Clustered.
2. Octave range: no note falls outside it, through fresh, breed, cross and edits by trait.
3. Time locks: notes starting in a locked range survive breed, cross and re-drawing unchanged; nothing new starts there; notes running in are trimmed.
4. Mutation is audible: every child differs from its parent, at the lowest spread too.
5. Sound choice: a Bass pad is chosen for low ranges when kept; nothing kept means no sound; a cleared sound is re-chosen on breeding.
6. Bars: switching 2 to 4 repeats; 4 to 2 cuts; the player loops on the right length against the host's position.
7. Keeping leaves both litters as they are; New resets both spreads.
8. Saving and loading gives everything back; anything else is rejected.

## How to build and check

- Work on branch `claude/plugin-album-collection-5nvyi5`. Commit and push there.
- `cmake -S plugins/bloodline-synth-2 -B build-s2 -G Ninja -DBLOODLINE_SYNTH2_PLUGIN=OFF && cmake --build build-s2 && ctest --test-dir build-s2 --output-on-failure` for the core. The full build fetches JUCE 9.0.5.
- Check the layout by rendering the editor to a PNG under `xvfb-run` (a scratch program that makes the processor, keeps a few sounds and loops, and calls `createComponentSnapshot`), and look at it before committing.
- CI: tests, pluginval at strictness 10, Mac VST3 and AU (`auval -v aumu Syn2 Jnic`).
- The README: install steps, how to play it, the traits table, and a checklist for the user, such as: Which traits did you use, and which did nothing? Two, four or eight bars: which found keepers? Did time locks let you keep the good half of a loop? Did the breeder choose sensible sounds? One thing to keep, one to cut, one to add.

## Out of scope

Time signatures other than 4/4; swing (later, if wanted, as the warp in Breaks 2); more than one sound a loop; layering synth sounds; chord symbols or a chord track; MIDI out; changes to `plugins/bloodline-synth` or `plugins/shared`.
