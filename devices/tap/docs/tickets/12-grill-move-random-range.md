# 12: Random range

**Status:** built 2026-10-26. `npm test` passes. Not yet tested in Live. Replaces ticket 09 (Wander).

**Goal:** a second knob, Effect Range, that gives every release a random Mod near the one that is set. The user liked a slow LFO on Mod drifting between -2 and +2: a balanced range, landing anywhere within it, with keys never running off the keyboard too quickly.

## The rule

- **Effect Range** is the total width of the range, centred on Mod. It goes from 0 to 24, in 0.01 steps, with a default of 0.
- On every release, in every mode, TAP picks a fresh number from `-range / 2` to `+range / 2`. Every number in that span is equally likely. There is no seed and no slow walk.
- The mode uses `mod + that number` as its Mod for that release, clamped to ±12.
- Effect Range 0 changes nothing, so the device works as it does now until the knob is turned up.
- With the pedal, each queued release picks its own number when the pedal lifts.

## What it does in each mode

- **Move,** Mod 0, Effect Range 2: each release moves the key anywhere from -1 to +1.
- **Gravity,** Mod 2, Effect Range 2: each release pulls as if Mod were anywhere from 1 to 3. If Effect Range is more than twice Mod, a release sometimes pushes.
- **Swap,** Mod 2, Effect Range 2: each release rotates 2, 3 or 4 keys (Mod 1 to 3 rounded, plus 1), and a negative result turns the other way. A result that rounds to 0 changes nothing.

## Tasks

- `tap.js`, test-first:
  - `createEngine` takes an optional random function, `Math.random` by default. The tests pass in a fixed one.
  - `range` setting (0 to `2 × MAX_MOD`, default 0) with `setRange`, `range()` and a Max handler `range`. `sync` reads it.
  - In `apply`, work out the release's Mod once (`clamp(mod + (random() - 0.5) × range, -MAX_MOD, MAX_MOD)`) and pass it to Swap, Gravity and Move in place of `mod`.
  - With Effect Range 0, do not call the random function, so every existing test still passes unchanged.
- Tests:
  - Effect Range 0 gives the same pitches as before and never calls the random function.
  - With a random function that returns 0, 0.5 and just under 1, Move at Mod 0, Effect Range 2 moves by -1, 0 and just under +1.
  - Mod plus the random amount stops at ±12.
  - Gravity with a large Effect Range sometimes pushes; Swap's count changes with the random amount.
  - The pedal picks one number per queued release.
  - Effect Range clamps to 0 to 24, and setting it changes no pitch and does not update the drift view.
  - The ES5 check still passes.
- Patch:
  - A `live.dial` **Effect Range** next to Mod (0 to 24, initial 0, 2401 steps so it moves in 0.01s, Float unit style), through its own `prepend range` into the `js`.
  - Update the patch tests: Effect Range is wired through its `prepend`.
  - Regenerate `TAP.amxd`.
- Mark ticket 09 as replaced by this ticket.

## Prototype

The patch changes, so drag the device in again.

1. `git pull`, then `npm test --prefix m4l`. The last lines show `# fail 0`.
2. **Knob:** Effect Range sits next to Mod, at 0.00. With it at 0, every mode works as before.
3. **Move,** Mod 0, Effect Range 2: release one key again and again. In the drift view it wanders up and down, never moving more than a semitone each time.
4. **Move,** Mod 1, Effect Range 1: each release moves up by somewhere between half a semitone and a semitone and a half.
5. **Gravity,** Mod 1, Effect Range 4: keys mostly pull together, but some releases push apart.
6. **Swap,** Mod 2, Effect Range 2: the number of keys passed round changes from release to release, with no stuck or silent notes.
7. **Pedal:** hold it, release several keys, then lift. Each key moves by its own amount.
8. **Saved with the set:** Effect Range saves with the set and can be automated.

## Done when

- Tests pass, including the ES5 check and the patch checks.
- The user has played Effect Range in all three modes in Live.
