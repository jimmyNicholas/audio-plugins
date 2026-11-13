# Kick Breeder spike

A throwaway Max for Live **instrument** for Live 11 (Max 8). It answers one question:

> If you choose between kicks round after round, with new ones bred from each choice, do you land on a kick you like within about ten rounds?

Every sound is synthesised: a swept sine body, a noise click, a filtered noise layer, compression, saturation and a low-pass. No samples. Spec: [`../docs/SPEC.md`](../docs/SPEC.md).

## Files

| File | What it is |
| --- | --- |
| `Kick Breeder Spike.amxd` | The device. Generated; load this one. |
| `Kick Breeder Spike.maxpat` | The same patch as readable JSON. Generated. |
| `kick-breeder.js` | What the device loads. Generated from `kick-voice.js` and [`shared/breeder/`](../../../shared/breeder/README.md) (the breeding engine and Max glue, shared with Snare Breeder). Must sit next to the `.amxd`. |
| `kick-voice.js` | The kick's settings, envelopes and loudness simulation (ES5). Edit this one. |
| `build-patch.mjs` | Writes the three generated files, with the kick's MSP voice: `npm run build`. |
| `tests/` | Node tests for the engine and the patch: `npm test`. |

## How the device works

- **Round 1** has slots A and B. **Hear** plays a slot; **Choose** keeps that kick as the favourite and breeds two variations, so later rounds have A, B and C. The favourite moves between slots unless Shuffle is off.
- **Variation** (the slider) sets how different the variations are, and breeds them again when you move it. Choosing the favourite again also moves it down, as you are close.
- **Save** puts the kick you last heard into one of four saved slots, which are saved with the set. **Play** plays a saved kick.
- **Breed this** starts a new search around the kick you last heard (a saved one, for example).
- **Locks** keep a setting as it is in new variations. **Back** undoes a round. **Reset** starts again at round 1 with the starting variation and no locks; saved kicks stay.
- **Match level** plays every kick at about the same loudness (from a rough simulation in the js). **Show fav** marks the favourite's slot.

### Keys

Live's note names, where C3 is middle C (MIDI 60).

| Keys | Do |
| --- | --- |
| C3, D3, E3 | Play slot A, B, C |
| F3, G3, A3, B3 | Play saved 1, 2, 3, 4 |
| C2, D2, E2 | Choose A, B, C |
| F2, G2, A2, B2 | Save the kick you last heard to 1, 2, 3, 4 |
| Any other note | Play the kick you last heard, so a drum clip on C1 still works for listening in a beat |

Velocity sets the level. Note off is ignored: each kick rings out until the next one.

### The kick

| Setting | Range |
| --- | --- |
| Pitch | 30 to 120 Hz |
| Sweep, Sweep time, Curve | 0 to 48 semitones above Pitch, falling over 5 to 200 ms, straight to snappy |
| Decay | 50 to 1500 ms |
| Click, Click tone | Level 0 to 1; band-pass 500 Hz to 12 kHz |
| Noise, Noise decay, Noise tone | Level 0 to 1; 10 to 1000 ms; low-pass 200 Hz to 12 kHz |
| Comp | 0 to 1: brings the tail of the body and noise up against the hit |
| Sat | 0 to 1: soft clipping (tanh) |
| Tone | Low-pass over everything, 200 Hz to 16 kHz |

**Compression** works on the envelopes, not the audio: it is worked out in the js like a compressor with a fixed threshold (-24 dB) and a ratio from 1:1 to 10:1, with make-up gain. It cannot pump or distort, and it leaves the click alone, like a compressor with a slow attack. A real signal compressor would need `gen~` or an object whose controls could not be checked here.

## Set up

1. In Live's browser, **Places > Add Folder**, and choose this `spike` folder in your checkout.
2. Make a **MIDI track** and drag **Kick Breeder Spike.amxd** onto it. It is an instrument, so it goes where Simpler or Operator would.
3. Make a one-bar MIDI clip with a note on every beat (any pitch). Loop it.
4. For errors: click the device's **Edit** button (the small icon in its title bar) and open **Window > Max Console** in Max. Copy any red lines into your notes.

## Checks

Fill in the Pass column (y / n / a note) and send it back.

| # | Do | Expect | Pass |
| --- | --- | --- | --- |
| 1 | Load the device | The status line reads `Round 1 \| choices 0 \| variation 0.25 \| hearing A`. Slot C reads `E3 (none this round)`. Saved 1 to 4 read `empty`. No red lines in the Max Console | [ ] |
| 2 | Press **Hear** on A, then B | Two different kicks. The line under the status describes each one | [ ] |
| 3 | Play **C3** and **D3** on your keyboard | The same two kicks | [ ] |
| 4 | Play the clip, then press C3 and D3 while it runs | The clip plays whichever slot you played last | [ ] |
| 5 | Play **C2** (or press **Choose** on A) | Round 2, and E3 now plays slot C | [ ] |
| 6 | Turn on **Show fav**. Choose the slot marked favourite | Variation drops from 0.25 to 0.15, and the slider moves | [ ] |
| 7 | Drag **Variation** up, then down, and hear the slots | High: very different kicks. Low: nearly the same | [ ] |
| 8 | Light **Decay** under Locks. Choose a few times, watching the description | Decay stays the same | [ ] |
| 9 | Find a kick with a lot of **Noise**, and one with high **Comp** and **Sat** | You can hear the noise layer, the longer, fuller tail, and the grit | [ ] |
| 10 | Turn **Match level** off for a few rounds, then on | Off: some kicks are much louder. On: about the same level | [ ] |
| 11 | Hear a kick, then play **F2** (or press **Save** on 1) | Saved 1 shows the kick, and **F3** plays it | [ ] |
| 12 | Fill saved 2 to 4 with **G2**, **A2**, **B2**; play **G3**, **A3**, **B3** | Each plays its saved kick | [ ] |
| 13 | **Back**, then **Reset** | Back: the previous round. Reset: round 1, variation 0.25, locks off, saved kicks still there | [ ] |
| 14 | Play a saved kick, then **Breed this** | A new search with that kick as the favourite | [ ] |
| 15 | Save the set, close it, open it again | Saved 1 to 4 are still there and play the same kicks | [ ] |
| 16 | Play a fast clip (16th notes) for a while | Note any clicks between hits, or hits that feel late | [ ] |

## The spike's answer

Do a few full searches from **Reset** to **Save** and note:

| Run | Choices to Save | How it felt (too slow, too random, stuck, good) |
| --- | --- | --- |
| 1 | | |
| 2 | | |
| 3 | | |

Do the C2 to E2 and F2 to B2 keys for choosing and saving feel right, or would another layout be better?

Also worth a line each:

- Did you trust your choice more with **Match level** on?
- Did knowing the favourite (**Show fav**) pull you towards it?
- Is anything missing from the kick itself: a sub, a second body layer, longer clicks?
- Is the envelope compression enough, or do you want a real compressor on the audio?

## Known limits

- **Not for a Drum Rack yet.** A Drum Rack sends C3 to every pad's instrument by default, and C3 plays slot A here. Put the device on its own MIDI track for now.
- The breeding rounds are not saved with the set. Saved kicks are.
- Loudness matching is based on a simulation of the patch, so levels are close, not exact.
- One voice: a new hit restarts the kick, which can click if the last one is still sounding.
- Saved kicks can't be tweaked with knobs. Use **Breed this** with a small Variation instead.
