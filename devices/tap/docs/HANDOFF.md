# Handoff: TAP as a Max for Live device

Last updated 2026-10-12 (ticket 02 built, waiting for the test in Live). Branch: `claude/codebase-vst-ableton-puxbqz`.

## Goal

Turn The Annoying Piano web app into a Max for Live MIDI effect that retunes notes and sends per-note pitch (MPE) to the instrument after it on the same track. Plan: [`SPEC.md`](SPEC.md). Tickets: [`tickets/`](tickets/). Research: [`RESEARCH.md`](RESEARCH.md).

## Where we are

| Ticket | State |
| --- | --- |
| 01 MPE spike and repo setup | **Done.** Passed in Live on 2026-10-11; results below. |
| 02 Engine, channels and Swap | **Built**, tests pass in Node. Waiting for the test in Live (steps below). |
| 03 to 05 | Not started |

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

## Testing ticket 02 in Live

1. `git pull` in Cursor, then `npm test --prefix m4l`. The last lines should show `# fail 0`.
2. On a MIDI track, load Wavetable, then drag **Places > m4l > TAP.amxd** to its left. Take the spike off the track.
3. Play A, release it, then play B and release it. Play A again: it sounds at B's pitch, and B sounds at A's.
4. Hold the sustain pedal and repeat with two other keys: nothing changes until the pedal lifts.
5. Play fast and hold more than 15 notes at once. When you let go, nothing should keep sounding.
6. If the device shows an error, open it in the Max editor and copy the red lines from the Max Console.

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

1. The user tests ticket 02 in Live (steps above) and the results are recorded here.
2. Then ticket 03: Gravity, Move and the panel ([`tickets/03-modes-panel.md`](tickets/03-modes-panel.md)).
