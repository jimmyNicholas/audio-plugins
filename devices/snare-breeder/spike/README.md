# Snare Breeder spike

A throwaway Max for Live **instrument** for Live 11 (Max 8), the snare version of the [Kick Breeder spike](../../kick-breeder/spike/README.md). It answers two questions:

> Can a snare built from plain MSP objects sound good enough to keep?
>
> Do you land on a snare you like within about ten rounds, as with the kick?

Every sound is synthesised: two swept sines for the drum skin, band-passed noise for the snare wires, a high-passed noise crack, compression, saturation and a low-pass. No samples. Spec: [`../docs/SPEC.md`](../docs/SPEC.md).

The panel, keys, saving and breeding are the same as the kick's, and come from the same code ([`shared/breeder/`](../../../shared/breeder/README.md)). Only the sound is new.

## Files

| File | What it is |
| --- | --- |
| `Snare Breeder Spike.amxd` | The device. Generated; load this one. |
| `Snare Breeder Spike.maxpat` | The same patch as readable JSON. Generated. |
| `snare-breeder.js` | What the device loads. Generated from `snare-voice.js` and `shared/breeder/`. Must sit next to the `.amxd`. |
| `snare-voice.js` | The snare's settings, envelopes and loudness simulation (ES5). Edit this one. |
| `build-patch.mjs` | Writes the three generated files, with the snare's MSP voice: `npm run build`. |
| `tests/` | Node tests for the voice and the patch: `npm test`. |

## How the device works

The same as the kick: **Hear** and **Choose** slots A, B and C, **Variation**, **Save** to four saved slots, **Breed this**, **Locks**, **Back**, **Reset**, **Match level** and **Show fav**. See the [kick's README](../../kick-breeder/spike/README.md#how-the-device-works) for each one.

### Keys

The same as the kick. Live's note names, where C3 is middle C (MIDI 60).

| Keys | Do |
| --- | --- |
| C3, D3, E3 | Play slot A, B, C |
| F3, G3, A3, B3 | Play saved 1, 2, 3, 4 |
| C2, D2, E2 | Choose A, B, C |
| F2, G2, A2, B2 | Save the snare you last heard to 1, 2, 3, 4 |
| Any other note | Play the snare you last heard, so a clip on D1 still works for listening in a beat |

### The snare

| Setting | Range | What you hear |
| --- | --- | --- |
| Pitch | 120 to 400 Hz | Where the skin settles: low and tom-like, or high and tight |
| Sweep, Sweep time | 0 to 24 semitones above Pitch, falling over 5 to 80 ms | The "dow" at the start of the hit |
| Ratio, Ring | A second tone at 1.3 to 2.4 times Pitch; level 0 to 1 | The skin's ring: none is a plain thump, high is metallic |
| Body decay | 30 to 500 ms | How long the skin sounds |
| Balance | 0 is all skin, 1 is all wires | Tonal snare at one end, a noise burst at the other |
| Wires decay | 40 to 800 ms | Short and tight, or long and washy |
| Wires tone, Width | Band-pass 800 Hz to 10 kHz; wide to narrow | Where the hiss sits; narrow sounds pitched and ringing |
| Bursts, Spread | 1 to 4 hits of the wires, 4 to 20 ms apart | 1 is a snare. 3 or 4 is a clap. Half of all random snares have 1 |
| Snap | 0 to 1 | A short, bright crack on top (high-passed noise at 2.5 kHz) |
| Comp | 0 to 1 | Brings the tails up against the hit |
| Sat | 0 to 1 | Soft clipping (tanh) |
| Tone | Low-pass over everything, 1 to 16 kHz | Dark and lo-fi, or open |

**Compression** works on the envelopes, as in the kick: see its README.

**Match level** is weighted for brightness, unlike the kick's. Ears hear 2 to 5 kHz as louder than the same level lower down, so plain matching would make bright snares louder, and they would keep winning. The simulation lifts everything above about 1.5 kHz by 4 dB before measuring, a rough stand-in for the weighting LUFS meters use.

## Set up

1. In Live's browser, **Places > Add Folder**, and choose this `spike` folder in your checkout.
2. Make a **MIDI track** and drag **Snare Breeder Spike.amxd** onto it. It is an instrument, so it goes where Simpler or Operator would. Use its own track, not a Drum Rack pad (see Known limits).
3. Make a one-bar MIDI clip with notes on beats 2 and 4 (any pitch). Loop it. A Kick Breeder track playing beats 1 to 4 next to it makes the best test.
4. For errors: click the device's **Edit** button (the small icon in its title bar) and open **Window > Max Console** in Max. Copy any red lines into your notes.

## Checks

Fill in the Pass column (y / n / a note) and send it back.

| # | Do | Expect | Pass |
| --- | --- | --- | --- |
| 1 | Load the device | The status line reads `Round 1 \| choices 0 \| variation 0.25 \| hearing A`. Slot C reads `E3 (none this round)`. Saved 1 to 4 read `empty`. No red lines in the Max Console | [ ] |
| 2 | Press **Hear** on A, then B | Two different snares. The line under the status describes each one | [ ] |
| 3 | Play **C3** and **D3**, and play the clip while you do | The same two snares, and the clip plays whichever you played last | [ ] |
| 4 | Choose a few times, until the description says `1 burst` | A snare, not a clap | [ ] |
| 5 | Choose until it says `3 bursts` or `4 bursts` | It sounds like a clap, or close to one | [ ] |
| 6 | Light **Balance** under Locks, and find one near 0 and one near 1 (watch the description) | Near 0: a pitched thump, all skin. Near 1: all hiss | [ ] |
| 7 | Find a snare with a high **Ring**, and one with a narrow **Width** (the description shows Ring; Width is not shown, so just listen for a pitched, ringing hiss) | A metallic ring in the skin; a whistling, pitched hiss in the wires | [ ] |
| 8 | Turn **Match level** off for a few rounds, then on | Off: some snares are much louder. On: about the same. Do bright snares still sound louder than dark ones? | [ ] |
| 9 | Save a snare to **F2**, close the set, open it again, play **F3** | The same snare | [ ] |
| 10 | Play a fast clip (16th notes) for a while | Note any clicks between hits, or hits that feel late | [ ] |

## The spike's answer

Do a few full searches from **Reset** to **Save** and note:

| Run | Choices to Save | How it felt (too slow, too random, stuck, good) |
| --- | --- | --- |
| 1 | | |
| 2 | | |
| 3 | | |

Also worth a line each:

- **Does it sound like a snare?** Which kind came closest (808, 909, acoustic, clap, lo-fi), and what's missing?
- Are claps in the same device useful, or do they get in the way of finding a snare?
- Are sixteen settings too many to breed quickly? Which ones never seemed to matter?
- Does the skin need a third tone, or the wires a second filter?

## Known limits

- **Not for a Drum Rack yet.** A Drum Rack sends C3 to every pad's instrument by default, and C3 plays slot A here. Put the device on its own MIDI track for now.
- The breeding rounds are not saved with the set. Saved snares are.
- Loudness matching is based on a simulation of the patch, so levels are close, not exact. The wires filter in the simulation is not Max's `svf~` exactly, so a very narrow Width may be matched less well.
- One voice: a new hit restarts the snare, which can click if the last one is still sounding. A long clap tail is cut by the next hit.
- Saved snares can't be tweaked with knobs. Use **Breed this** with a small Variation instead.
