# 02: Engine, channels and Swap

**Goal:** the first "annoying" behaviour, on the real device.

## Tasks
- `m4l/tap.js`, ES5, test-first with `node --test`:
  - Pitch table of 128 keys, each stored as semitones from MIDI note 0 (so concert pitch is the note number). `reset`.
  - `noteReleased(note)` applying Swap, with the first-release rule.
  - Sustain pedal queue: while the pedal is down, queue released keys; when it lifts, release them in order.
  - MPE channel logic ported from `plugin/Source/MpeRouter.cpp` and its tests: channels 2 to 16, bend before note on, note off on the matching channel, shared messages on channel 1, steal the oldest note, reuse the channel free longest.
  - Max glue at the bottom (`msg_int`/`list` handlers, `outlet`), kept thin. Node only sees the exports.
- Test cases taken from `__tests__/lib/data/modes/swap.test.ts`, `__tests__/utils/hertzHelpers/`, `plugin/tests/MpeRouterTests.cpp` and the parked engine tests at commit `89272e4`.
- `m4l/TAP.maxpat` and the generated `m4l/TAP.amxd`: `midiin` to `js tap.js` to `midiout`, `is_mpe` on.
- Apply ticket 01's results (Bend Range setting or plain-object note path, if needed).

## Prototype
Load `TAP.amxd` before Wavetable. Play A then B and release both: A now sounds at B's pitch. With the pedal held, nothing changes until it lifts. No stuck notes after fast playing or holding more than 15 notes.

## Done when
- `npm test --prefix m4l` passes here and in CI.
- Swap behaves like the web app for the same key sequence.
