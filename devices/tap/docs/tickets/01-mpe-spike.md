# 01: MPE spike and repo setup

**Goal:** prove that per-note pitch from a Max for Live MIDI effect reaches Wavetable on the same track in Live 11, measure the bend range, and judge the `js` delay. This is the riskiest assumption, so nothing else is built first.

## Tasks

### Repo setup
- Create `m4l/` with `tests/` and `tools/`.
- `m4l/tools/amxd.mjs`: `pack` wraps a `.maxpat` into an unfrozen MIDI effect `.amxd` (chunks `ampf` `mmmm`, `meta`, `ptch`); `unpack` does the reverse. Node built-ins only. Round trip tested against a known file.
- Add `.github/workflows/m4l-tests.yml`: runs `node --test m4l` on pushes that touch `m4l/`.
- Change `.github/workflows/plugin-macos.yml` to run only when started by hand (`workflow_dispatch`).
- Mark `docs/vst/` as replaced by `docs/m4l/` at the top of its spec and handoff.

### Spike device (`m4l/spike/`, throwaway)
- `TAP MPE Spike.maxpat` and the generated `.amxd`: `midiin` into `js tap-spike.js` into `midiout`, with `is_mpe` on. A `live.numbox` Detune control (-100 to +100 cents) and a `live.tab` Keys choice (black keys only, all keys).
- `tap-spike.js` (ES5, `autowatch = 1`): each note on gets the next free channel from 2 to 16, a 14-bit pitch bend for the detune (48 semitone range), then the note. Note off goes out on the note's channel. Everything else passes through on channel 1.
- Unit tests for the bend maths and the channel round trip.

### Instructions for you
- Step by step: add `m4l/` to Places, load the spike before Wavetable, and, only if Live will not load the generated file, open it in the Max editor and save it.

## Prototype (what you check in Live)
1. **Independent pitch:** Keys on black only, Detune +50. Hold a white key, add a black key. The white key must not move.
2. **Bend range:** add Live's Tuner after Wavetable. Detune +100, play C#. The Tuner should read D, within a few cents.
3. **Timing:** Detune 0. Play fast passages and chords. Say whether it feels any later than with the device turned off.
4. Optional, only if check 1 fails: turn off "Patch supports MPE" in the Patcher Inspector and see whether anything changes.

## Done when
- Checks 1 to 3 are reported back and recorded in the handoff.
- If check 1 fails, stop and diagnose before ticket 02.
- If check 2 is off, ticket 02 adds a Bend Range setting.
- If check 3 shows a noticeable delay, ticket 02 plans the per-note path in plain Max objects.
