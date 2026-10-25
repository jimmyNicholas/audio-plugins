# 09: Harmony

**Goal:** the released key snaps to a pure interval from the last one.

## The mode
- Take the interval from last's pitch to current's pitch. Keep the whole octaves, and replace the rest with the nearest ratio from the chosen set. Current is retuned to that interval above (or below) last.
- **Ratios**, a `live.tab`:
  - **Just** (default): one ratio for every semitone, 1/1, 16/15, 9/8, 6/5, 5/4, 4/3, 45/32, 3/2, 8/5, 5/3, 9/5, 15/8, 2/1. Corrections are small, up to about 20 cents.
  - **Pure:** only 1/1, 4/3, 3/2 and 2/1. Keys snap to octaves, fourths and fifths, so most intervals jump.
- On the very first release last is current, so nothing changes.
- Each key is tuned against the key before it, so pure intervals pile up and the piano drifts away from concert pitch over a phrase (the comma pump). That is the point of the mode.

## Check with the user first
- The two ratio sets. Other choices: a 7-limit set with 7/4 and 7/6, or a Pure set with thirds.

## Tasks
- `tap.js`, test-first:
  - `HARMONY = 7`, `setRatios`.
  - Tests: a tempered fifth (700 cents) becomes 702.0 cents, a major third becomes 386.3, intervals below last and more than an octave away, Pure snapping a minor second to unison, and the first release.
- Panel: a `live.tab` Ratios (Just, Pure), saved with the set and automatable, read by `sync`.
- Regenerate the device. You will need to drag it in again.

## Prototype
- **Harmony**, Just: play C then G, releasing each. G is now 2 cents sharp. Play C, E, G, C, E, G in turn: the chord sounds smoother than equal temperament and slowly slides off concert pitch.
- Pure: play a scale. Keys jump onto fourths, fifths and octaves of each other.

## Done when
- Tests pass, including the ES5 check.
- Ratios survives saving and reopening the set.
