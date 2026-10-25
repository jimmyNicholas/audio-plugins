# 08: Wander

**Goal:** each released key drifts at random.

## The mode
- The released key moves by a random amount between `-range` and `+range` cents, the same chance for every amount in between. Last plays no part.
- **Range** runs 1 to 100 cents, default 20. The result is kept within 0 to 127.
- Acts on the very first release too, as Move does.

## Tasks
- `tap.js`, test-first:
  - `WANDER = 6`, `setRange`.
  - `createEngine` takes an optional random function, `Math.random` by default. The tests pass in a seeded one, so every run gives the same pitches.
  - Tests: moves stay within the range, the same seed gives the same moves, and over many releases moves land on both sides.
- Panel: a `live.numbox` Range in cents, saved with the set and automatable, read by `sync`.
- Regenerate the device. You will need to drag it in again.

## Prototype
- **Wander**, Range 20: play one key over and over. It slowly drifts up and down. Each key goes its own way. The drift view shows faint reds and blues spreading.
- Range 100: keys wander by up to a semitone at a time.

## Done when
- Tests pass, including the ES5 check.
- Range survives saving and reopening the set.
