# Handoff: TAP as a Max for Live device

Last updated 2026-10-12 (ticket 05 built, waiting for a test in Live). Branch: `claude/codebase-vst-ableton-puxbqz`.

## Goal

Turn The Annoying Piano web app into a Max for Live MIDI effect that retunes notes and sends per-note pitch (MPE) to the instrument after it on the same track. Plan: [`SPEC.md`](SPEC.md). Tickets: [`tickets/`](tickets/). Research: [`RESEARCH.md`](RESEARCH.md).

## Where we are

| Ticket | State |
| --- | --- |
| 01 MPE spike and repo setup | **Done.** Passed in Live on 2026-10-11; results below. |
| 02 Engine, channels and Swap | **Done.** Passed in Live on 2026-10-12: every check below worked and `npm test` passed. |
| 03 Gravity, Move and the panel | **Done.** Passed in Live on 2026-10-12: the user reported that it works. |
| 04 Semitone mode | **Done.** Passed in Live on 2026-10-12: the user reported that it works. |
| 05 Drift view | **Built, not yet tested in Live.** `npm test` passes. |
| 05.2 Picture mode | Planned (user's idea, 2026-10-12). Not started. |

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

## Ticket 04: what was built

- `tap.js`: `createSemitoneRouter`, with the same interface as the MPE router, and an Output setting that picks between them.
  - Semitone sends plain note on, note off and poly aftertouch on channel 1, each pitch rounded to the nearest note (0 to 127). No bends. Swap and Move stay exact because their pitches are whole notes; Gravity is rounded.
  - Two held keys on the same note: the second key restarts the note (a note off, then its own note on), so the instrument never has two voices on one note. The note off goes out only when the last of the keys is released.
  - Changing Output stops every held note through the old output, so nothing sticks. Those keys still count as released when they come up, as with a stolen note. Changing Output does not reset the pitches.
  - There is no voice limit in Semitone, so no stealing.
- `TAP.maxpat` and `TAP.amxd`: a `live.tab` Output (MPE, Semitone; default MPE) under Reset, saved with the set and read by `sync` on load.
- "Patch supports MPE" stays on. Notes on channel 1 should reach a non-MPE instrument such as Operator as ordinary notes; the test in Live confirms it.

## Testing ticket 04 in Live

The patch changed, so drag the device in again.

1. `git pull` in Cursor, then `npm test --prefix m4l`. The last lines should show `# fail 0`.
2. Make a MIDI track with **TAP.amxd** then **Operator**. The panel now has an Output tab under Reset. Set it to **Semitone**.
3. **Swap:** play A, release, play B, release. Play A again: it sounds as B.
4. **Move**, Semitones 1: release a key again and again. It goes up a semitone each time.
5. **Gravity**, Strength 5: play the A below middle C, release, play the A above it, release. Play the low A again: it sounds as middle C (rounded from slightly sharp of C).
6. **Shared note:** Gravity, Strength 10. Play C, release, play E, release. Hold C and E together: both sound E. Let go of C: E keeps sounding. Let go of E: it stops.
7. **No stuck notes:** play fast in each mode, with and without the pedal, for a while. Nothing keeps ringing.
8. **Switching:** hold a chord and change Output. The chord stops and nothing sticks. Back on a Wavetable track with Output on MPE, everything works as in ticket 03.
9. **Saved with the set:** Output saves with the set like the other controls.
10. If the device shows an error, open it in the Max editor and copy the red lines from the Max Console.

## Ticket 05: what was built

- The user chose a pop-out window over a wider panel.
- `tap.js`: a second outlet sends a list of 128 offsets in cents (each key's drift from concert pitch, to 0.1 cent) after each change to the pitch table: once per release, once when the pedal lifts with releases waiting, and on Reset or a mode change. Nothing is sent on note on, and a note off goes out before its table. `sync` also sends it, so the view is filled on load.
- `m4l/drift-view.js`: a `jsui` drawing all 128 keys. Keys turn red when sharp and blue when flat, deeper the further they drift, with a bar above each key. The scale is logarithmic up to an octave so a few cents still show. Hovering over a key or its bar shows its name (Live's names, middle C is C3), its offset and the note it now plays near. With no hover, the top line counts the drifted keys.
- `TAP.maxpat` and `TAP.amxd`: a **Drift view** button next to Reset (not a Live parameter, so it is not automatable) sends `open` to `pcontrol`, which opens the `p Drift` subpatcher as a 960 by 200 window. Data reaches it through `s ---drift` and `r ---drift`, so two TAP devices each get their own view. The view keeps its table while the window is closed.
- Tests: the drift table and when it is sent, the view's layout, colours, hover and drawing against a fake `mgraphics`, and a check that `tap.js` and `drift-view.js` use only ES5 syntax.

## Testing ticket 05 in Live

The patch changed, so drag the device in again.

1. `git pull` in Cursor, then `npm test --prefix m4l`. The last lines should show `# fail 0`.
2. MIDI track with **TAP.amxd** then **Wavetable**. Click **Drift view** next to Reset. A window opens with a keyboard and "Every key at concert pitch".
3. **Move**, Semitones 1: play and release middle C a few times. C3 turns red, deeper each time, with a growing bar. Hover over it: "C3 +300.0 cents (plays near D#3)" after three releases.
4. **Gravity**, Strength 2: play some keys. Keys turn faintly red or blue. Hover to read the cents.
5. **Swap:** release A then B. Both change colour, one sharp and one flat.
6. **Pedal:** hold the pedal and play. Nothing changes in the view until the pedal lifts, then it updates in one go.
7. **Reset** (and changing Mode) clears the view.
8. **Speed:** play the same fast passages as ticket 01, with the view open. Notes should feel just as tight as before.
9. Close and reopen the window: it shows the current drift, not a blank keyboard.
10. If the window does not open or shows an error, open the device in the Max editor and copy the red lines from the Max Console.

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

1. Test ticket 05 in Live (steps above).
2. Then ticket 05.2: picture mode in the drift window ([`tickets/05.2-picture-mode.md`](tickets/05.2-picture-mode.md)).
3. After that, the only item left in the spec is "later: save the pitch table with the set".
