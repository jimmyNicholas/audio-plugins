# 10: Harmony

**Status:** paused (2026-10-25). Ticket 11 takes it out of the device. The idea comes back after the core three and the panel.

**Goal:** the released key is retuned to a pure interval from the last one.

## The mode
- Mod chooses the interval, in semitones from last: +7.00 is a perfect fifth up, -2.00 a major second down. Current is retuned to exactly that pure interval from last's pitch.
- Mod is rounded to the nearest whole semitone. Each semitone uses one pure ratio, with whole octaves added for intervals past 12:

| Semitones | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Ratio | 1/1 | 16/15 | 9/8 | 6/5 | 5/4 | 4/3 | 45/32 | 3/2 | 8/5 | 5/3 | 9/5 | 15/8 | 2/1 |

- Downward intervals use the same ratios turned over: -7 puts current a 3/2 below last.
- Mod 0 puts current exactly on last's pitch, like Gravity at Mod 10.
- On the very first release last is current, so nothing changes. Results stay within the bounds.
- Each key is tuned against the key before it, so over a phrase the intervals pile up and the piano drifts away from concert pitch.

## Tasks
- `tap.js`, test-first:
  - `HARMONY = 7`.
  - Tests: +7 gives 702.0 cents above last, +4 gives 386.3, -2 gives 203.9 below, +19 gives an octave and a fifth, Mod 6.40 rounding to 6, Mod 0, the first release, and the bounds.
- No patch change.

## Prototype
- **Harmony**, Mod 7.00: play C, release, then play any key and release it. Play that key again: it sounds a pure fifth above C. Keep going: each key lands a fifth above the one before.
- Mod 4.00: each key lands a pure major third above the one before, 14 cents flat of an equal-tempered third.

## Done when
- Tests pass, including the ES5 check.
