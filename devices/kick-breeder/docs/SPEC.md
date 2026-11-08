# Spec: Kick Breeder

Working name. A Max for Live instrument for Ableton Live 11 that synthesises a kick drum with no samples. You find the kick you want by choosing between candidates, round after round, instead of turning knobs. When you are happy, you keep it and it plays on MIDI like any drum.

Background: [Composing from nothing, without samples](../../../docs/research/sample-free-composition-tools.md), section 5 ("Breeder drum"). The idea is interactive evolution, as in Synplant's seeds and Microtonic's Patternarium, narrowed to one kick and run locally inside Live.

Status: idea. A throwaway Max for Live spike is in [`spike/`](../spike/README.md): an instrument that runs the choosing loop, with a checklist to fill in. The engine in `spike/kick-breeder.js` is ES5 and meant to lift into the real device. Decisions marked **(proposed)** are the builder's recommendation and still need the user's yes.

## The loop

1. **Start:** two kicks are shown, **A** and **B**, made from random settings within kick-like ranges.
2. **Listen:** click a candidate to hear it, or play MIDI to hear the highlighted one (see "Listening in context").
3. **Choose:** pick the one you prefer.
4. **Next round:** the winner stays, and two new kicks are bred from it. Three candidates are shown: the winner and two variations.
5. **Repeat** until you like what you hear.
6. **Keep:** the chosen kick becomes the device's sound. MIDI notes play it, its settings save with the Live set, and the breeding panel can be put away.

So round one is a choice from two, and every round after is a choice from three, with the current favourite always one of them.

## How variations are bred

Each kick is a set of settings (see "The kick voice"), each stored as a number from 0 to 1. A variation is the winner with every unlocked setting nudged by a random amount, kept inside 0 to 1.

- **Step size:** how far a nudge can go. It starts wide (proposed: 0.25 of each range).
- **Converging (proposed):** if you pick the winner again, you are close, so the step shrinks (times 0.6, down to a floor of 0.02). If you pick a new variation, the step stays the same, so the search keeps moving.
- **Wider and Closer buttons:** override the step by hand, for "these are all too similar" or "too different".
- **Locks:** each setting has a lock. A locked setting is copied unchanged into every variation, so you can fix the decay you like and breed only the punch.
- **Fresh:** throws away the current search and starts again from two random kicks.
- **Back:** goes to the previous round. The device keeps a history of every winner in the session.
- **Seeded randomness (proposed):** the random numbers come from a seeded generator, so a round can be replayed and the engine can be tested.

## Fair listening

A/B choices are easily fooled. Two guards:

- **Matched loudness (proposed):** every candidate plays at the same loudness, so you choose on character, not volume. A louder kick nearly always "wins" otherwise. How to measure loudness without `gen~` is the first open question below.
- **Shuffled slots (proposed, with a toggle):** the winner is not always in slot A. Its position is shuffled each round, so you are not just picking "the one on the left". A toggle shows which slot is the current favourite, for when you want to know.

## Listening in context

A kick that sounds great alone can be wrong in the track. While breeding:

- **Click** a slot: plays that candidate once, at full velocity.
- **MIDI in:** notes coming into the track (for example, a running drum clip) play the **highlighted** slot. Switching the highlight while the loop runs is the real A/B test.
- **Choose:** a separate button per slot, so highlighting to listen and choosing to breed are not the same action.

## Keep

- Press **Keep**: the highlighted candidate's settings are written to the device's main controls (`live.*` objects), so they save with the set, can be automated and can be stored as a Live preset.
- After Keep, MIDI notes play the kept kick. Velocity sets level (and, proposed, a little click and drive, as on a real drum).
- **Breed from here:** starts a new search with the kept kick as the first winner, for later tweaks.
- **Pitch from note (proposed, a toggle):** off by default, so every note plays the kick at its own pitch. On, the note number transposes it, for 808-style tuned kicks.

## The kick voice

Built from plain MSP objects (no `gen~`): a sine with a pitch envelope, a short click, drive and a tone filter.

| Setting | What it does | Range (proposed) |
| --- | --- | --- |
| Pitch | Where the body settles | 30 to 120 Hz |
| Sweep | How far above Pitch the hit starts | 0 to 48 semitones |
| Sweep time | How fast the pitch falls | 5 to 200 ms |
| Sweep curve | Straight or snappy fall | 0 to 1 |
| Decay | Body length | 50 to 1500 ms |
| Click level | Transient loudness | 0 to 1 |
| Click tone | Transient brightness | 500 Hz to 12 kHz |
| Drive | Saturation | 0 to 1 |
| Tone | Low-pass over the whole kick | 200 Hz to 20 kHz |

Nine settings is small enough to breed quickly and wide enough to cover deep sub kicks, short clicky kicks and distorted ones.

## Technical approach

Follows TAP's pattern ([TAP spec](../../tap/docs/SPEC.md), "Technical approach"):

- **Max 8**, bundled with Live 11. No `gen~` (the licence on hand cannot edit it). No third-party externals.
- **Breeding engine in `js`, ES5:** settings, mutation, step size, locks, history, slot shuffle and the seeded random generator live in one file that also exports itself for Node, so it is unit tested with `node --test` and checked as ES5 in CI.
- **Thin patch:** the patch holds the kick voice, the panel and the wiring. Nothing that can be tested lives in the patch.
- **Instrument device:** Kick Breeder makes sound from MIDI, so it is an instrument, not an effect. `tools/amxd.mjs` packs it with `--type instrument` (`iiii`).
- **Patch kept as JSON** (`.maxpat`) and packed to an unfrozen `.amxd`; both committed.

## Open questions

1. **Loudness matching without `gen~`:** the spike simulates each kick in the `js` and sets a gain from that. Is it close enough by ear, or does it need rendering into a `buffer~` and measuring?
2. **How many rounds feel right?** If it takes more than about ten choices to land a kick, the step size rules need tuning. The user's listening decides.
3. **Choice from three, or from more?** Three keeps choices fast. A "show four" option could come later.
4. **Are nine settings enough,** or does it need a second body oscillator or a sub layer?
5. **Name:** Kick Breeder is a placeholder.

## Later, not now

- The same loop for snares, hats and toms, then a whole kit: the "kit from one seed" idea in the research note.
- Export the kept kick as a WAV, for use outside Live.
- Feed it from TAP so a tuned kick drifts as you play.
