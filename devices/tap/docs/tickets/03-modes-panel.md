# 03: Gravity, Move and the panel

**Goal:** all three modes, controlled from the device.

## Tasks
- Add Gravity (strength 1 to 10, with strength 10 pulling exactly onto the current key) and Move (-2 to 2 semitones) to `tap.js`, test-first from `__tests__/lib/data/modes/`.
- Changing mode resets every key. Add `reset` from a button.
- Panel: `live.tab` Mode (Swap, Gravity, Move), `live.dial` Strength, `live.numbox` Semitones, `live.text` Reset. All saved with the set and automatable. The `js` reads its settings from these controls.
- Regenerate the device. You will need to drag it in again.

## Prototype
Each mode behaves as in the web app. Change mode and the keys reset. Save the set, reopen it: the mode and settings are kept, and the pitches start at concert pitch.

## Done when
- Tests pass.
- Settings survive saving and reopening the set, and can be automated.
