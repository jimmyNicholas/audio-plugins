# Handoff: TAP as a Max for Live device

Last updated 2026-10-27 (tickets 11 to 15 built, not yet tested in Live). The work is merged to `main` ([#58](https://github.com/jimmyNicholas/next-annoying-piano/pull/58)). Tickets 01 to 05.2 were built on `claude/codebase-vst-ableton-puxbqz`. Tickets 06 to 10 are on `claude/gifted-goldberg-1g5nfb`, branched from `main`.

## Goal

Turn The Annoying Piano web app into a Max for Live MIDI effect that retunes notes and sends per-note pitch (MPE) to the instrument after it on the same track. Plan: [`SPEC.md`](SPEC.md). Tickets: [`tickets/`](tickets/). Research: [`RESEARCH.md`](RESEARCH.md).

## Where we are

| Ticket | State |
| --- | --- |
| 01 MPE spike and repo setup | **Done.** Passed in Live on 2026-10-11; results below. |
| 02 Engine, channels and Swap | **Done.** Passed in Live on 2026-10-12: every check below worked and `npm test` passed. |
| 03 Gravity, Move and the panel | **Done.** Passed in Live on 2026-10-12: the user reported that it works. |
| 04 Semitone mode | **Done.** Passed in Live on 2026-10-12: the user reported that it works. |
| 05 Drift view | **Done.** Passed in Live on 2026-10-12: the user reported that it works. |
| 05.2 Picture mode | **Done.** Passed in Live by 2026-10-20: the user reported that it works. |
| 06 Mode menu, Mod knob and bounds | **Built, not yet tested in Live.** `npm test` passes. |
| 07 Gravity on Mod, Repel and Echo | **Built, not yet tested in Live.** `npm test` passes. Test together with 06. |
| 08 to 10 Rotate, Wander, Harmony | **Paused** (2026-10-25). Rotate becomes part of Swap in ticket 11. |
| 11 Back to the core three | **Built, not yet tested in Live.** `npm test` passes. Replaces 06 and 07 in Live testing. |
| 12 Random range | **Built, not yet tested in Live.** `npm test` passes. Replaces 09. Test after 11. |
| 13 Chance | **Built, not yet tested in Live.** `npm test` passes. Test after 12. |
| 14 Panel window | **Built, not yet tested in Live.** `npm test` passes. Test after 13. [Mock-up](https://claude.ai/artifact/EopL6dHcj3qczfJ9DH9Azs). |
| 15 Square visualiser | **Built, not yet tested in Live.** `npm test` passes. Test after 14. |
| 16 to 18 | **Parked.** Double slider for the bounds; floor at the same note name; other views. |

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

## Ticket 05.2: what was built

- The user's idea: a pixel art picture that scrambles as the piano drifts. They chose built-in art (a landscape) and a Keys / Picture switch in the existing window.
- `drift-view.js` only. No change to `tap.js`, the patch or the device file.
  - `LANDSCAPE`: 16 by 8 letters, one per note, note 0 top left, coloured from `LANDSCAPE_COLOURS` (sky, cloud, sun, snowy mountain, hills, lake, trees, field).
  - `slotSquares(slot, cents)`: each key's slot shows the square for the pitch it now plays. Whole note drift shows another square (Swap trades two, Move shifts one); drift between notes blends the two nearest squares; pitches past either end show the end square.
  - Keys and Picture buttons at the right of the header. The choice lasts while the device is loaded; it is not a Live parameter.
  - Hover in Picture mode outlines the square and shows the same text as in Keys mode.
- Tests: the mapping, the layout, the switch, a swap in the drawn picture, and hover.

## Testing ticket 05.2 in Live

Only `drift-view.js` changed, and it uses `autowatch`, so a loaded device picks it up after `git pull`. If the window looks unchanged, drag the device in again.

1. `git pull` in Cursor, then `npm test --prefix m4l`. The last lines should show `# fail 0`.
2. Open **Drift view** and click **Picture** at the top right. A landscape appears: sky, cloud, sun, mountain, hills, lake and trees.
3. **Swap:** release keys. Squares trade places in pairs and the picture scrambles.
4. **Move**, Semitones 1: release one key a few times. Its square steps along to the next squares.
5. **Gravity**, Strength 2: squares smear into the colours of their neighbours.
6. **Reset** or a mode change puts the picture back together.
7. Hover over a square: the top line names its key and drift. Click **Keys** to go back to the keyboard.
8. **Speed:** fast passages with Picture showing feel as tight as before.

## Ticket 06: what was built

- `tap.js`:
  - Mode numbers for all eight modes, in menu order: Swap 0, Gravity 1, Move 2, Repel 3, Echo 4, Rotate 5, Wander 6, Harmony 7. `setMode` rounds and clamps to 0 to 7. Modes 3 to 7 change no pitch yet, but a release still counts as the last release and still updates the drift view, so tickets 07 to 10 only add a rule in `apply`.
  - `mod` replaces `semitones`: a float from -12 to 12, default 1. Move adds it, so Move now takes fractions and up to an octave.
  - `lowest` and `highest` (0 to 127, defaults 0 and 127). `bound(pitch)` keeps every pitch a mode writes within them, reading them the other way round if Lowest is above Highest. Swap and Gravity use it too. Setting a bound changes no pitch and does not update the drift view.
  - Gravity's overshoot below 0 Hz now goes to Lowest instead of note 0.
  - Max handlers `mod`, `lowest` and `highest`; `semitones` is gone. `sync` reads `mod`, `lowest` and `highest`.
- `TAP.maxpat` and `TAP.amxd`:
  - Mode is a `live.menu` with the eight names. It keeps the old `live.tab`'s id, `varname` and parameter name `Mode`.
  - Semitones, its label and its `prepend` are gone.
  - New: `live.dial` Mod (-12 to 12, initial 1, 2401 steps so it moves in 0.01s, Float unit style), and `live.numbox` Lowest and Highest (0 to 127, MIDI note unit style) with labels. Each goes through its own `prepend` into the `js`.
  - Layout: Mode menu under the title; the Mod dial (where Strength was); Lowest and Highest to its right; Reset, Drift view and Output where they were.
- Tests: Mod and the bounds in the engine, the device and the Max glue; modes 3 to 7 changing nothing; and two new checks on `TAP.maxpat` itself: every control is wired through its `prepend` into the `js`, and the Mode menu lists the eight modes in engine order.

## Ticket 07: what was built

- The user's change: Mod's positive and negative range is for strength and repel, so **Gravity reads Mod** and the Strength dial is gone.
- `tap.js`: one `gravity(last, current, strength)` for both modes; Gravity passes `mod` and Repel `-mod`. A positive strength pulls, a negative one pushes. `share(strength)` gives the part of the gap moved: the web's `1 / (10 - s)` from 1 to 10, `s / 9` below 1 (so Mod 0 does nothing), and a full pull or push at 10 and above. A full pull lands on the current key; a full push goes to the bound in the direction away from it. Keys already at one pitch do not move.
- `echo(last, current)`: current gets last's drift plus Mod, bounded. On the first release it just moves by Mod.
- `setStrength`, `strength()`, the `strength` handler and its `sync` entry are gone.
- `TAP.maxpat` and `TAP.amxd`: the Strength dial and `prepend strength` are gone; Mod sits where Strength was, with Lowest and Highest beside it.
- Tests: Gravity's web values with Mod as strength, negative Mod, Mod 0 and below 1, above 10, Repel against Gravity, full pushes to each bound, one pitch, Echo up and down, bounds; the patch check now also confirms Strength is gone.

## Testing tickets 06 and 07 in Live

The patch changed, so drag the device in again.

1. `git pull` in Cursor, then `npm test --prefix m4l`. The last lines should show `# fail 0`.
2. MIDI track with **TAP.amxd** then Wavetable (or one of the Waves pianos). The panel now has a Mode dropdown, a Mod dial, and Lowest and Highest boxes. Strength and Semitones are gone.
3. **Mode:** the dropdown lists Swap, Gravity, Move, Repel, Echo, Rotate, Wander, Harmony. Swap works as before. Rotate, Wander and Harmony change nothing yet.
   - **Gravity**, Mod 2.00: as before at Strength 2. Mod -2.00: keys push apart instead. Mod 10.00: the last key lands exactly on the current one.
   - **Repel**, Mod 5.00: play A3 (the A below middle C) then the A above it a few times, releasing each. They push apart until they reach Lowest and Highest. Mod -5.00 pulls like Gravity.
   - **Echo**, Mod 0.10: play a rising scale once. Each key is 10 cents sharper than the one before (check with Drift view). Play it again and the trail carries on.
4. **Move**, Mod 7.00: each release moves the key up a fifth. Mod -0.50: down a quarter tone (on MPE; Semitone output rounds it).
5. **Mod dial:** check it shows two decimals and can be set to 0.01 steps (hold Shift while dragging, or type a value).
6. **Bounds**, Highest C3 (middle C, as Live names it): with Move at +2.00, release a key below middle C again and again. It stops at middle C. Set Lowest above Highest: the bounds still work, read the other way round.
7. **Saved with the set:** Mode, Mod, Lowest and Highest save with the set and reopen as they were.
8. **Old sets:** open a set saved with the old device on Gravity or Move. Does it open on the same mode? (Unknown: the Mode control changed from a tab to a menu.) The Strength and Semitones values are lost and Mod opens at 1.00.
9. If the device shows an error, open it in the Max editor and copy the red lines from the Max Console.

## Ticket 11: what was built

- `tap.js`:
  - Three modes only: Swap 0, Gravity 1, Move 2. Repel and Echo, their rules and tests are gone, and `setMode` clamps to 0 to 2.
  - The engine keeps the newest 13 releases (`releases`, oldest first). The same key released twice in a row does not join it. Reset and a mode change clear it.
  - Swap rotates the newest N places, N = round(|Mod|) + 1, or fewer while the list fills. Positive Mod: each place takes the next place's pitch, the newest takes the oldest's. Negative: the other way. Pitches are read first, then written oldest to newest, so a key's newest place wins. At Mod ±1 it matches the old Swap release for release.
  - Gravity: `share = GRAVITY_AT_ONE × |Mod|^log2(GRAVITY_PER_DOUBLING)`, with the constants at the top of the file (0.01 and 3) for ticket 14. Positive pulls, negative pushes, in Hertz; a push below 0 Hz goes to Lowest. The old 1 to 10 strength curve is gone.
- `TAP.maxpat` and `TAP.amxd`: Mode is a horizontal three-step `live.slider` (enum Swap, Gravity, Move), keeping the id, `varname` and parameter name `Mode`.
- Found while building: the ticket's worked example (release C, E, G at Mod +2 to get E, G, C) only holds from concert pitch, because E's release already swaps C and E. The rule was kept and the example and test step corrected in the ticket.

## Testing ticket 11 in Live

The patch changed, so drag the device in again. Steps 1 to 8 of the prototype in [`tickets/11-grill-core-three.md`](tickets/11-grill-core-three.md). Also check the Mode slider shows the name (Swap, Gravity, Move) and fits above the Mod dial; it has not been seen in Max yet.

## Ticket 12: what was built

- `tap.js`:
  - `createEngine(onChange, random)`: `random` stands in for `Math.random`, so the tests fix the picks.
  - `range` (Effect Range): 0 to 24, default 0, with `setRange`, `range()`, a Max handler `range` and a `sync` entry. Setting it changes no pitch and does not update the drift view.
  - `releaseMod()`: once per release, `clamp(mod + (random() - 0.5) × range, -12, 12)`, passed to Move, Gravity and Swap in place of `mod`. With Effect Range 0 it returns Mod without calling `random`, so the device works as before. Queued pedal releases each pick their own number when the pedal lifts.
- `TAP.maxpat` and `TAP.amxd`: a `live.dial` beside Mod, parameter name **Effect Range** (short name **Range**, so it fits under the dial), 0 to 24, initial 0, 2401 steps, Float unit style, through `prepend range` into the `js`. Lowest and Highest moved right to make room.
- Tests: Effect Range 0 matches the old pitches in all three modes and never calls `random`; Move's span; centred on Mod; the ±12 stop; Gravity pushing; Swap's count and direction; the pedal; the setting's limits; the Max glue; the patch wiring.

## Testing ticket 12 in Live

The patch changed, so drag the device in again. Steps 1 to 8 of the prototype in [`tickets/12-grill-move-random-range.md`](tickets/12-grill-move-random-range.md). Also check the dial reads **Range** and fits beside Mod, with Lowest and Highest still readable to its right; the layout has not been seen in Max yet.

## Ticket 13: what was built

- `tap.js`:
  - `chance`: 0 to 100, rounded, default 100, with `setChance`, `chance()`, a Max handler `chance` and a `sync` entry. Setting it changes no pitch and does not update the drift view.
  - `fires()`: once per release, `random() < chance / 100`. At 100 and 0 it does not call `random`, so the device works as before at 100.
  - In `apply`, the release is recorded first (Swap's list, Gravity's last key), then Chance is rolled. A miss skips the mode's rule and picks no Effect Range number; the drift view is still sent. Queued pedal releases each roll when the pedal lifts, in order.
- `TAP.maxpat` and `TAP.amxd`: a `live.dial` **Chance** beside Effect Range, 0 to 100, initial 100, 101 steps, Percent unit style, through `prepend chance` into the `js`. Lowest and Highest moved right again to make room; ticket 14 lays the strip out properly.
- Tests: Chance 100 matches the old pitches in all three modes and Chance 0 changes nothing, neither calling `random`; 0.29 fires and 0.3 misses at 30; a miss joins Swap's list and becomes Gravity's last key; a miss picks no Effect Range number; the pedal; the view after a miss; the setting's limits; the Max glue; the patch wiring.

## Testing ticket 13 in Live

The patch changed, so drag the device in again. Steps 1 to 9 of the prototype in [`tickets/13-grill-chance.md`](tickets/13-grill-chance.md). Also check the dial reads **Chance** and fits beside Effect Range, with Lowest and Highest still readable to its right; the layout has not been seen in Max yet.

## Ticket 14: what was built

- `tap.js`:
  - `modLabel()`: the line under Mod. Swap: "Keys passed round: N", or "No swap" when Mod rounds to 0. Gravity: "Pull: X% of the gap" or "Push: X% of the gap", to a tenth of a percent. Move: "+2.00 semitones each release", with "0.00" and no sign at 0. A third outlet sends `set <label>` on each change to Mode, Mod or Gravity's tuning, and on `sync`; not on releases.
  - `howItWorks()`: the mode's name and the ticket's draft text. A fourth outlet sends `name set <name>` and `text set <text>` on each change to Mode and on `sync`. The texts live in `tap.js`, not in the patch, because a message box splits its text at each comma and turns "+1" into the number 1.
  - Gravity tuning: `setGravityAtOne(percent)` (0 to 100, default 1) and `setGravityPerDoubling(factor)` (1 to 10, default 3), with Max handlers `gravityAtOne` and `gravityPerDoubling`. They change no pitch, do not update the drift view and are not read by `sync`. They do resend the label, so prototype step 7 reads 2%.
- `TAP.maxpat` and `TAP.amxd`:
  - The strip, 360 by 169 (`devicewidth` 360, so Live does not size it from the rightmost control): Mode, then Mod, Effect Range and Chance in a row, then Reset and **Open** on the left; Lowest, Highest and Output on the right. **Open** (`varname` `open`, not a parameter) replaces Drift view and sends `open` to `pcontrol`.
  - `p Drift` is now `p Panel`, 920 by 320, in presentation mode: Mode, Reset and Output; How it works (the name and text comments, fed by `r ---how` into `route name text`); Modifiers (Mod, Effect Range and Chance, the Mod label comment fed by `r ---label`, Lowest and Highest); and a black 256 square `panel` for ticket 15. The old `jsui` is gone from the patch; `drift-view.js` and its tests stay until ticket 15 rewrites them. `s ---drift` is kept, though nothing receives it until then.
  - Window copies are plain Max objects, `varname` `window-<name>`: `tab` for Mode and Output, `flonum` for Mod (-12 to 12) and Effect Range (0 to 24), `number` for Chance (0 to 100) and for Lowest and Highest (note names, middle C as C3), and a `button` for Reset. The first build used `live.*` copies with their parameter off, as the ticket asked; in Live 11's Max 8.1 they showed blank and passed nothing on either way, so the window now has no `live.*` objects at all. Each copy sends to `s ---<name>-window`, received next to its strip control. Each strip control goes through `prepend set` to `s ---<name>-strip`, received at its copy, so the copy never sends the value back. Reset is one way only: the window's Reset bangs the strip's.
  - `live.thisdevice` bangs every strip control except Reset on load, so the copies start in step with the saved values.
  - **Debug** (a `toggle`) sends `window size` to `thispatcher`: 320 tall closed, 430 open. The debug controls sit below 320, so closing Debug hides them. Pull at Mod 1 and Per doubling (`flonum`s) go through `prepend gravityAtOne` and `prepend gravityPerDoubling` to `s ---tuning`, received at the `js`. View size (128, 256, 384, 512) and Header summary (Off, On), both `tab`s, go through `prepend size` and `prepend summary` to `s ---visualiser`, for ticket 15 to receive. `loadmess set` shows the defaults (1, 3 and 256) on load.
- `tap.js`'s load-time `sync` waits for Max's scheduler (a `Task`), since Max makes the outlets only after the script has run. Sending straight away logged `bad outlet index` in the Max Console; `live.thisdevice` synced again just after, so nothing was lost.
- Tests: each mode's label, including the ticket 11 table, "No swap" and Move's signs; the label and How it works are sent on the right changes and not on releases; tuning at 2% and ×2 gives 8% at Mod 4; tuning limits, no pitch change, not read on load; the load-time sync waits for the scheduler. Patch checks: the strip size, Open and `pcontrol`, the window size, no `live.*` objects in the window, each copy has its strip control's range and names and is linked both ways, the load bangs, the label and How it works wiring, Debug's sizes and the tuning's route into the `js`.
- Not yet known, check in Live:
  - `window size` puts the window back at 120, 120 each time Debug is clicked.
  - How it works for Gravity says "triples" even after Debug changes Per doubling. Draft text; say if it should follow the setting.
  - Pull at Mod 1 is not capped against the curve: at 2% and ×3, Mod 12 asks for over 100% of the gap and the last key passes the current key. Debug only; say if it should be capped.

## Testing ticket 14 in Live

The patch changed, so drag the device in again. Steps 1 to 8 of the prototype in [`tickets/14-grill-panel-window.md`](tickets/14-grill-panel-window.md). The layout has not been seen in Max yet: check the strip fits, the How it works text wraps inside its section and the window copies show their names and ranges. If the device shows an error, open it in the Max editor and copy the red lines from the Max Console.

## Ticket 15: what was built

- `drift-view.js`, rewritten: the Keys and Picture views, hover and their constants are gone. Now a square pitch map on black: across is the key, up is the pitch it plays, both note 0 at the bottom left.
  - `squareLayout(size, header)`: pixels per note (`size / 128`) and the square's top left, 18 pixels down when the header is shown.
  - `keyRect(note, cents, layout)`: the key's pixel on the diagonal. `stemRect` runs from it to the exact pitch played (`note + cents / 100`): up from the pixel's top when sharp, down from its bottom when flat, so Move +3 at 256 is a 6 pixel stem. None when under a pixel. A stem stops at the square's edge, so a key pushed past note 127 does not draw over the header.
  - `keyColour(cents)`: white under 0.1 cent, else full red or full blue.
  - Max handlers `size` (snaps to the nearest of 128, 256, 384 or 512, and takes the tab's text as a symbol) and `summary` (0 or 1). Each resizes the `jsui` from its top left corner, in the window (`presentation_rect`) and in the patcher, and redraws. The header line reads "Every key at concert pitch", "1 key drifted" or "12 keys drifted". `list` is unchanged.
- `TAP.maxpat` and `TAP.amxd`: the black `panel` in the window is now the `jsui` (`varname` `window-view`), 256 square at the same place. `r ---drift` and `r ---visualiser` feed it. View size now sends its tab's text (the middle outlet) instead of its index, so `size 256` arrives, not `size 1`.
- Tests: concert pitch is 128 white pixels on the diagonal and no stems; Move +3 on note 60; a Swap pair; the 0.1 cent edge; each size and snapping; the box resize; the header on and off with its counts; stems clipped at the edge; a short list ignored; the ES5 check. Patch checks: one `jsui` with `drift-view.js` in the window, fed the drift table, View size by its text and Header summary.
- Not yet known, check in Live:
  - Resizing uses `box.setattr("presentation_rect", ...)` and `box.rect`; not yet seen working in Max 8.1.
  - At 384 and 512 the square runs past the bottom of the window (320, or 430 with Debug open) and over the debug controls. Debug only; resize the window by hand to see it all.
  - At 128 the header line may be cut off at its right.

## Testing ticket 15 in Live

The patch changed, so drag the device in again. Steps 1 to 9 of the prototype in [`tickets/15-grill-square-visualiser.md`](tickets/15-grill-square-visualiser.md). Then tell me which size to keep, so the Size setting can go.

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
- **Vercel** fails on these branches for reasons unrelated to this work. The user does not need the web app on Vercel, so ignore it.
- **No PR** unless the user asks.
- **User preferences:** Australian spelling, no em dashes, small reviewable tickets, review before committing, ask before big changes.

## New direction (2026-10-25)

The user is not confident about how the new modes were added and wants to pause them, rework Swap, Gravity and Move, and build a better pop-out panel. Each idea has a grilling ticket: a list of questions, each with a recommended answer, to settle with the user before any code. Grill in this order, because the later ones depend on the first:

1. [11 Back to the core three](tickets/11-grill-core-three.md)
2. [12 Random range](tickets/12-grill-move-random-range.md) (replaces ticket 09; grilled)
3. [13 Chance](tickets/13-grill-chance.md) (grilled)
4. [15 Square visualiser](tickets/15-grill-square-visualiser.md) (built)
5. [14 Panel window](tickets/14-grill-panel-window.md) (grilled)

Once a ticket is grilled, rewrite it as a build ticket with Tasks, Prototype and Done when.

## Next steps

1. Test tickets 11 to 15 in Live, and pick a visualiser size.
2. Whether tickets 06 and 07 still need testing in Live depends on ticket 11.
3. Later: save the pitch table with the set.

Hosting an instrument inside TAP was explored and is not needed: TAP already works with Waves Grand Rhapsody Piano and Electric 88 placed after it on the same track (which Output was used is not yet recorded).
