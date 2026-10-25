# 06: Mode menu, Mod knob and bounds

**Goal:** a panel with room for eight modes, one knob that every mode reads, and a pitch range keys cannot leave.

Part of "three modes to eight" (tickets 06 to 10). Rules for all the new modes: [`../SPEC.md`](../SPEC.md#more-modes).

## Decided with the user (2026-10-25)
- **Mode** becomes a dropdown (`live.menu`) listing all eight modes: Swap, Gravity, Move, Repel, Echo, Rotate, Wander, Harmony. New modes are added after Move, so sets saved with the first three keep their mode.
- **Mod**, one global knob from -12.00 to +12.00 in semitones, in steps of 0.01. Each mode reads it in its own way (table in the spec).
- **Lowest** and **Highest**, global note bounds. Every mode's result is kept within them.

## Choices made here (change them if they feel wrong)
- **Move reads Mod**, so the Semitones box goes. Move can now shift by up to an octave either way, and by fractions: Mod 0.50 moves a quarter tone. A set saved with Semitones opens with Mod at its default.
- **Strength** was kept as its own dial at first. Changed with the user in ticket 07: Gravity and Repel read Mod, and the Strength dial goes.
- **Mod's default is 1.00**, which keeps Move's old default.
- **Bounds:** Lowest defaults to note 0, Highest to 127. If Lowest is set above Highest, the two are read the other way round. Changing a bound does not move any key by itself: keys outside the new range come back inside the next time a mode changes them. So turning a knob never retunes anything.
- Gravity's old rule of holding pitches at note 0 becomes the Lowest bound.
- Modes not yet built (Repel to Harmony until their tickets) change nothing on release.

## Tasks
- `tap.js`, test-first in `tests/tap.test.js`:
  - `setMod`, `setLowest`, `setHighest`, and one `bound(pitch)` used by every mode.
  - Move reads Mod. `setSemitones` goes.
  - `setMode` accepts 0 to 7.
  - Tests: Move with whole and fractional Mod, Move held at Highest, Gravity held at Lowest, bounds read the other way round, changing a bound not changing the drift table.
- Panel:
  - `live.menu` Mode, `live.dial` Mod (two decimals), `live.numbox` Lowest and Highest shown as note names. Reset, Output and Drift view stay. All saved with the set, automatable, and read by `sync`.
  - Check everything fits the device height (169 pixels).
- Regenerate the device. You will need to drag it in again. After this ticket, tickets 07 to 10 change only `tap.js`, so `autowatch` picks them up after `git pull`.

## Prototype
- Choose modes from the dropdown. Swap and Gravity work as before.
- **Move**, Mod 7.00: each release moves the key up a fifth. Mod -0.50: down a quarter tone.
- **Bounds**, Highest C4: with Move at +2.00, release a key below C4 again and again. It stops at C4.

## Done when
- Tests pass, including the ES5 check.
- Mode, Mod, Lowest and Highest survive saving and reopening the set, and a set saved before this ticket opens on the same mode.
