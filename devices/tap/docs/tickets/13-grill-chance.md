# 13: Chance

**Status:** built 2026-10-27. `npm test` passes. Not yet tested in Live.

**Goal:** a Chance knob that sets how likely each release is to fire. At 30%, about three releases in ten change pitch fully and the rest change nothing. The user's idea was a dry / wet "chance to fire" for a mode.

## The rule

- **Chance** goes from 0 to 100%, in 1% steps, with a default of 100%. One knob for every mode, like Mod and Effect Range.
- On every release, TAP rolls once. The release fires if `random() < chance / 100`.
  - At 100% it always fires and no roll is made, so the device works as it does now until the knob is turned down.
  - At 0% it never fires and no roll is made.
- **A miss still counts:** the release joins Swap's list and becomes Gravity's last key, as if the mode had nothing to do that time. No pitch changes.
- **Order:** Chance is rolled first. On a miss, no Effect Range number is picked.
- **Pedal:** each queued release gets its own roll when the pedal lifts, in order.
- **Drift view:** misses are not shown. The view still updates after every release, fired or not.

## What it does in each mode

- **Move,** Mod 1, Chance 50%: about half the releases move the key up a semitone. The rest leave it where it is.
- **Swap,** Mod +1, Chance 50%: release C, E (miss), then G. E stays put, and G swaps with E, not C.
- **Gravity,** Chance 50%: about half the releases pull. A miss still becomes the last key, so the next release pulls it.

## Tasks

- `tap.js`, test-first:
  - `chance` setting (0 to 100, rounded, default 100) with `setChance`, `chance()` and a Max handler `chance`. `sync` reads it.
  - In `apply`, after recording the release, roll Chance with ticket 12's random function, before the Effect Range number is picked. On a miss, skip the mode's rule but still send the drift table.
  - At 100% and 0%, do not call the random function, so every existing test still passes unchanged.
- Tests:
  - Chance 100 gives the same pitches as before and never calls the random function.
  - Chance 0 changes no pitch and never calls the random function.
  - At Chance 30, a random function returning 0.29 fires and 0.3 misses.
  - A miss joins Swap's list (C, E missed, G at Mod +1: G swaps with E) and becomes Gravity's last key.
  - A miss picks no Effect Range number.
  - The pedal rolls once per queued release.
  - The drift table is sent after a miss.
  - Chance clamps and rounds to 0 to 100, and setting it changes no pitch and does not update the drift view.
  - The ES5 check still passes.
- Patch:
  - A `live.dial` **Chance** next to Effect Range (0 to 100, initial 100, 101 steps, Percent unit style), through its own `prepend chance` into the `js`.
  - Update the patch tests: Chance is wired through its `prepend`.
  - Regenerate `TAP.amxd`.

## Prototype

The patch changes, so drag the device in again.

1. `git pull`, then `npm test --prefix m4l`. The last lines show `# fail 0`.
2. **Knob:** Chance sits next to Effect Range, at 100%. With it at 100%, every mode works as before.
3. **Move,** Mod 1, Chance 50%: release one key again and again. In the drift view it climbs a semitone on some releases and stays put on others.
4. **Chance 0%:** no mode changes any pitch.
5. **Swap,** Mod +1, Chance 50%: play a phrase. Some releases swap and some do not, with no stuck or silent notes.
6. **Gravity,** Mod 4, Chance 30%: keys pull together, but more slowly than at 100%.
7. **With Effect Range:** Move, Mod 0, Effect Range 2, Chance 50%: some releases wander, the rest stay put.
8. **Pedal:** hold it, release several keys, then lift. Only some of the keys move.
9. **Saved with the set:** Chance saves with the set and can be automated. An LFO on Chance works.

## Done when

- Tests pass, including the ES5 check and the patch checks.
- The user has played Chance in all three modes in Live.

At the end of each ticket, open a pull request into main.
