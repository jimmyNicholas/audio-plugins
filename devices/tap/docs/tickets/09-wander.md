# 09: Wander

**Status:** paused (2026-10-25). Ticket 11 takes it out of the device. Ticket 12 may replace it.

**Goal:** each released key drifts at random.

## The mode
- The released key moves by a random amount between `-mod` and `+mod` semitones, the same chance for every amount in between. The sign of Mod is ignored. Last plays no part.
- Mod 0.20 moves a key by up to 20 cents; Mod 12.00 by up to an octave. Mod 0 changes nothing.
- Acts on the very first release too, as Move does. Results stay within the bounds.

## Tasks
- `tap.js`, test-first:
  - `WANDER = 6`.
  - `createEngine` takes an optional random function, `Math.random` by default. The tests pass in a seeded one, so every run gives the same pitches.
  - Tests: moves stay within Mod, the same seed gives the same moves, over many releases moves land on both sides, a negative Mod behaves like a positive one, and the bounds.
- No patch change.

## Prototype
- **Wander**, Mod 0.20: play one key over and over. It slowly drifts up and down. Each key goes its own way. The drift view shows faint reds and blues spreading.
- Mod 1.00: keys wander by up to a semitone at a time.

## Done when
- Tests pass, including the ES5 check.
