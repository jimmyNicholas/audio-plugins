# Spec: The Annoying Piano as a Max for Live device

Replaces the VST plan in [`../vst/SPEC.md`](../vst/SPEC.md). Research behind this plan: [`RESEARCH.md`](RESEARCH.md).

## Goal

Turn The Annoying Piano (TAP) web app into a Max for Live MIDI effect for Ableton Live 11 Suite on an Intel Mac (macOS 13). It sits on a track before an instrument, retunes notes as you play and keeps TAP's core idea: every key you release changes the pitch of keys you play later.

TAP makes no sound itself. Ableton's instruments and effects provide the sound.

## Why Max for Live

The VST could not deliver per-note pitch: Ableton merges all MIDI channels into one when MIDI is routed from track to track, so the per-note bends of ticket 02 were lost. A Max for Live MIDI effect sits on the same track as the instrument, so its MIDI never crosses tracks.

## Behaviour to keep

Unchanged from the VST spec:

1. Every key starts at concert pitch (A4 = 440 Hz, equal temperament).
2. Pressing a key plays the pitch stored for that key. A sounding note does not change pitch while it is held.
3. Releasing a key applies the current mode, using the key just released (current) and the key released before it (last):
   - **Swap:** swap the stored pitches of last and current.
   - **Gravity:** `last = last - (last - current) / (10 - strength)`. Strength runs 1 to 10, default 2.
   - **Move:** shift current by N semitones, N from -2 to 2, default 1.
4. On the very first release, last equals current, so Swap and Gravity do nothing.
5. **Sustain pedal:** while the pedal is down, released keys are queued. When the pedal lifts, the mode is applied to each queued key in release order.
6. Changing mode resets every key to concert pitch.

## More modes

Tickets 06 to 10 take TAP from three modes to eight. The new modes are not in the web app. They follow the same rules as the first three: they act on release, the pedal queues them, and Reset or a mode change clears them. They are added after Move, so sets saved with the first three keep their mode.

Mode is a dropdown. One global **Mod** knob, -12.00 to +12.00 in semitones, is read by each mode in its own way. Global **Lowest** and **Highest** notes bound every mode's result; changing them moves no key until a mode next changes it.

| # | Mode | On release | Reads |
| --- | --- | --- | --- |
| 0 | Swap | Swap last and current | Nothing |
| 1 | Gravity | Last is pulled towards current (web formula, in Hertz, with Mod as strength); a negative Mod pushes it away instead | Mod |
| 2 | Move | Current moves by Mod semitones (was the Semitones box) | Mod |
| 3 | Repel | Gravity reversed: a positive Mod pushes last away from current, and 10 or more pushes it to a bound; a negative Mod pulls | Mod |
| 4 | Echo | Current takes last's drift, plus Mod | Mod |
| 5 | Rotate | The last N keys released pass their pitches round; N is Mod rounded, 2 to 12, and a negative Mod turns the other way | Mod |
| 6 | Wander | Current moves by a random amount up to Mod either way | Mod |
| 7 | Harmony | Current is retuned to the pure interval Mod from last: +7 is a 3/2 fifth up, -2 a 9/8 down | Mod |

For Gravity and Repel, Mod is the web app's strength: from 1 to 10 the share of the gap moved is `1 / (10 - strength)`, below 1 it fades in from nothing at 0, and 10 or more is a full pull onto the current key (or a full push to a bound). The Strength dial is gone.

## Deliberate changes from the web app

- **Gravity at strength 10:** the web formula divides by zero. TAP pulls the key exactly onto the current key's pitch.
- **Key range:** the full MIDI range 0 to 127.
- **Reset button:** puts every key back to concert pitch without changing mode.
- **Velocity:** passed through.
- **Input channel:** ignored. All incoming notes are treated alike. Controller pitch bend, mod wheel, pedal and other shared messages pass through on channel 1.
- **Sound:** comes from Ableton, so the web app's synth, vibrato and reverb are not rebuilt.

## How pitch reaches the instrument

- The device has "Patch supports MPE" (`is_mpe`) turned on, or Live collapses its MPE output onto one voice.
- Each note gets its own MIDI channel, 2 to 16 (up to 15 notes at once). Channel 1 carries shared messages.
- TAP sends a pitch bend on the note's channel, then the nearest whole note. The bend range is assumed to be 48 semitones, the MPE default. Ticket 01 measures whether Live agrees.
- Note off goes out on the same channel as its note on. When all 15 channels are busy, the oldest note is stopped. A new note takes the channel that has been free longest, so release tails are not cut off.
- MPE instruments in Live 11: Wavetable, Sampler, Simpler, and from 11.3 also Drift, Analog, Collision, Tension and Electric. Not Operator. Ticket 04's semitone mode covers non-MPE instruments.

## Technical approach

- **Max version:** Max 8, bundled with Live 11. Max 9 and its `v8` object do not work with Live 11.
- **Logic in `js`:** Max 8's `js` object runs roughly ES5. The engine (pitch table, modes, pedal queue, channel allocation) is hand-written ES5 in one file. The file also exports itself for Node, so it is unit tested with `node --test` here and in CI.
- **Thin patch:** the patch only wires `midiin` to `js` to `midiout` and connects the panel controls. All behaviour that can be tested lives in the `js` file.
- **Timing:** `js` runs at low priority, which adds a little delay to each note. Ticket 01 checks whether you can feel it. If you can, the per-note path moves into plain Max objects and `js` only updates the pitch table on release.
- **Settings:** panel controls are `live.*` objects, so they save with the set and can be automated. The `js` keeps no state that cannot be rebuilt from them, except the pitch table, which resets when a set is reopened.
- **Device file:** the patch is kept in git as readable JSON (`.maxpat`). A Node script wraps it into an unfrozen `.amxd` (a small binary header plus the JSON). Both are committed, so a `git pull` gives you a ready-to-load device. If Max rejects a generated file, you build that patch by hand from step by step instructions and the script unpacks it back to JSON. Frozen devices are never committed.

## Folder layout

```
m4l/
  TAP.amxd           generated device, loaded in Live from here
  TAP.maxpat         patch source
  tap.js             engine plus Max glue, ES5
  drift-view.js      jsui for the drift view window, ES5
  package.json       `npm test` and `npm run build`, no dependencies
  tests/             node --test files
  tools/amxd.mjs     pack .maxpat into .amxd, and unpack
  spike/             ticket 01's throwaway device
docs/m4l/            spec, tickets, research, handoff
```

## Development loop

1. Once: in Live's browser, Places > Add Folder, and choose `m4l/` in your local checkout. ("Patch supports MPE" is already set inside the device.)
2. Drag the device from Places onto a MIDI track before Wavetable.
3. After each ticket, `git pull` in Cursor. Because `tap.js` uses `autowatch`, a running device reloads its logic on its own. If the ticket changes the patch, it says so, and you drag the device in again.
4. `npm test --prefix m4l` (from the repo root) runs the engine tests in a second. It installs nothing; Node 18 or later is enough.

## Prototype rule

Every ticket ends in something you can try in Live. Each ticket is reviewed before it is committed, and you test it before the next ticket starts. Work stays on `claude/codebase-vst-ableton-puxbqz`; nothing is merged to `main`.

## Decisions

1. Max for Live MIDI effect on the same track as the instrument (replaces the VST).
2. Target Live 11 and Max 8.
3. For your own use for now. Kept reproducible so sharing later is easy.
4. v1: Swap, Gravity and Move, their settings and Reset, on a plain panel. Semitone mode and the drift view follow.
5. The pitch table is not saved with the set in v1.
6. Engine in hand-written ES5, tested in Node.
7. Note path in `js` unless ticket 01 shows the delay is noticeable.
8. `.amxd` generated from the repo, with hand-built as the fallback.
9. The VST in `plugin/` stays parked. Its macOS workflow runs only when started by hand.

## Tickets

| # | Ticket | What you test in Live |
| --- | --- | --- |
| 01 | [MPE spike and repo setup](tickets/01-mpe-spike.md) | Independent bends, bend range, timing |
| 02 | [Engine, channels and Swap](tickets/02-engine-swap.md) | Swap on Wavetable |
| 03 | [Gravity, Move and the panel](tickets/03-modes-panel.md) | All modes; settings save with the set |
| 04 | [Semitone mode](tickets/04-semitone-mode.md) | Operator plays rounded pitches |
| 05 | [Drift view](tickets/05-drift-view.md) | See keys drift as you play |
| 05.2 | [Picture mode](tickets/05.2-picture-mode.md) | A pixel art picture scrambles as keys drift |
| 06 | [Mode menu, Mod knob and bounds](tickets/06-mode-menu-mod-bounds.md) | Move by any amount; keys stop at the bounds |
| 07 | [Gravity on Mod, Repel and Echo](tickets/07-repel-echo.md) | Mod pulls or pushes; drift trails along a phrase |
| 08 | [Rotate](tickets/08-rotate.md) | Pitches pass round in threes, either way |
| 09 | [Wander](tickets/09-wander.md) | Keys drift at random |
| 10 | [Harmony](tickets/10-harmony.md) | Each key lands a pure interval from the one before |
| later | Save the pitch table with the set | |

Tickets 04 and 05 can swap places.

## Out of scope for now

- Sharing or publishing the device (freezing, user docs, Live 12 testing).
- Saving the pitch table with the set.
- Loading MIDI files inside the device.
- A built-in synth.
