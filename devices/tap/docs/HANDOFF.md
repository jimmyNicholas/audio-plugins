# Handoff: TAP as a Max for Live device

Last updated 2026-10-12 (ticket 03 built, waiting for review and a test in Live). Branch: `claude/codebase-vst-ableton-puxbqz`.

## Goal

Turn The Annoying Piano web app into a Max for Live MIDI effect that retunes notes and sends per-note pitch (MPE) to the instrument after it on the same track. Plan: [`SPEC.md`](SPEC.md). Tickets: [`tickets/`](tickets/). Research: [`RESEARCH.md`](RESEARCH.md).

## Where we are

| Ticket | State |
| --- | --- |
| 01 MPE spike and repo setup | **Done.** Passed in Live on 2026-10-11; results below. |
| 02 Engine, channels and Swap | **Done.** Passed in Live on 2026-10-12: every check below worked and `npm test` passed. |
| 03 Gravity, Move and the panel | **Built, not yet tested in Live.** Steps below. |
| 04 to 05 | Not started |

## Ticket 01 results (2026-10-11)

Tested by the user with Cursor's help ([`m4l/spike/CURSOR.md`](../../m4l/spike/CURSOR.md)). Live 11.0.10, bundled Max 8.1.11, Node 20.12.2.

| Check | Result |
| --- | --- |
| 1. Independent pitch | Pass: a held white key does not move when a detuned black key comes in. |
| 2. Bend range | Pass (reported as "T"): ±100 cents on C# reads as D and C, so Live reads per-note bend as ±48 semitones. No Bend Range setting needed. |
| 3. Timing | No lag felt. The note path stays in `js`. |
| Loading | The generated `.amxd` loaded straight away, no re-save in Max. Keep generating devices from the repo. |

Live 11.0.10 is before 11.3, so the only MPE instruments available are Wavetable, Sampler and Simpler.

## Ticket 02: what was built

- `m4l/tap.js`: pitch table (semitones from note 0), Swap with the first-release rule, sustain pedal queue, and the MPE channel logic from the spike and `MpeRouter.cpp`. Each note goes out as the nearest whole note plus a bend for the rest, so bends stay within half a semitone.
- `m4l/TAP.maxpat` and the generated `m4l/TAP.amxd`: `midiin` to `js tap.js` to `midiout`, `is_mpe` on. No controls yet; they come in ticket 03.
- Choices made along the way:
  - A note off for a key that is not down (say the device loaded while a key was held) is ignored, so it does not retune anything.
  - The same key played again while it is held stops the old note, and that does not count as a release.
  - A note stolen because all 15 channels were busy still counts as released when its key comes up.
  - All notes off (CC 123) and all sound off (CC 120) stop every note without counting as releases. The pedal queue is kept.

## Ticket 03: what was built

- `tap.js`: Gravity and Move, the Mode, Strength and Semitones settings, and Reset.
  - Gravity works in Hertz with the web app's formula, then turns the result back into a pitch. At strength 10 it puts the last key exactly on the current key's pitch.
  - Just below 10 the web formula overshoots a long way (strength 9.9 pulls ten times the gap), so a pitch that would fall below note 0 is held at note 0. The spec keeps the web formula, so this is unchanged; ask the user if it should be smoothed.
  - Move shifts the released key, including on the very first release, as in the web app.
  - Changing mode resets every key. Choosing the mode that is already on does not, so automation sending the same value again changes nothing. Changing Strength or Semitones never resets.
  - Reset and mode changes also drop releases still waiting for the pedal.
- `TAP.maxpat` and `TAP.amxd`: `live.tab` Mode, `live.dial` Strength (1 to 10, default 2), `live.numbox` Semitones (-2 to 2, default 1), `live.text` Reset. Each goes into the `js` through a `prepend`.
- Loading: `live.thisdevice` sends `sync`, which reads every control by its scripting name. `tap.js` also syncs when `autowatch` reloads it, so the panel and the engine stay in step after a `git pull`.
- The Max glue is tested too: the tests load `tap.js` in a sandbox with a fake patcher.

## Testing ticket 03 in Live

The patch changed, so drag the device in again.

1. `git pull` in Cursor, then `npm test --prefix m4l`. The last lines should show `# fail 0`.
2. Take the old TAP off the Wavetable track and drag **Places > m4l > TAP.amxd** in its place. The panel shows Mode, Strength, Semitones and Reset.
3. **Swap** still works as in ticket 02.
4. **Move**, Semitones 1: play a key and release it, again and again. It goes up a semitone each time. Set Semitones to -2: it comes down two at a time.
5. **Gravity**, Strength 2: play A, release, play the E above, release. Play A again: it is a little sharp of A. Set Strength to 10 and repeat with fresh keys: A now sounds the same as E.
6. **Mode resets:** retune a few keys, then change mode. Every key is back at concert pitch. Retune again, press **Reset**: the same, and the mode stays.
7. **Saved with the set:** choose Gravity, Strength 6, Semitones -1. Save the set, close it and open it again. The panel shows those values, Gravity is on, and every key starts at concert pitch.
8. **Automation:** in Arrangement view, pick TAP and Mode (and then Strength) in the automation lanes, draw a change and play it back. The panel follows.
9. If the device shows an error, open it in the Max editor and copy the red lines from the Max Console.

## Why not the VST

Ableton merges all MIDI channels into one when MIDI goes from track to track, so the VST's per-note bends never reached Wavetable (VST ticket 02). The VST in `plugin/` is parked; ask the user before deleting it. History: [`../vst/HANDOFF.md`](../vst/HANDOFF.md).

## The user's setup

- Ableton Live 11 Suite on an Intel Mac, macOS 13.7.8. Live 11 bundles Max 8.
- Uses Cursor locally. Loads the device straight from `m4l/` in the local checkout (added to Live's Places), and runs `npm test --prefix m4l`.
- Tests with Wavetable. Operator does not support MPE.

## Decisions (do not reopen without the user)

See "Decisions" in [`SPEC.md`](SPEC.md). In short: Live 11 and Max 8 only; for the user's own use; v1 is three modes, their settings and Reset on a plain panel; pitch table not saved with the set; ES5 engine tested in Node; note path in `js` unless ticket 01 shows a delay; `.amxd` generated from the repo, hand-built as the fallback.

## Open questions

None from ticket 01. Questions 1 to 5 from the earlier list are answered in the results above.

## Gotchas

- **`js` in Max 8 is ES5:** no `let`, arrow functions, classes, template literals, `Map` or `Promise`. Use `Task` for timers.
- **Channel 1** is the MPE master channel. Notes on it are not treated as per-note.
- **Pushing:** check `git log origin/<branch>..HEAD` first so nothing unexpected goes up.
- **Vercel** fails on every commit on this branch for reasons unrelated to this work. Do not merge to `main`.
- **No PR** unless the user asks.
- **User preferences:** Australian spelling, no em dashes, small reviewable tickets, review before committing, ask before big changes.

## Next steps

1. Review ticket 03, then test it in Live with the steps above.
2. Ticket 04: Semitone mode ([`tickets/04-semitone-mode.md`](tickets/04-semitone-mode.md)).
