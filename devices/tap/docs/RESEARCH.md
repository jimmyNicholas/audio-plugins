# Max for Live research: The Annoying Piano as an M4L MIDI effect (Live 11 Suite)

Researched 2026-10-10. Target: Ableton Live 11 Suite, Intel Mac, macOS 13, with a Max for Live MIDI effect feeding Wavetable on the same track.

## How to read this

- **Method caveat:** the sandbox's egress proxy blocked direct fetches of `cycling74.com`, `docs.cycling74.com`, `help.ableton.com` and `ableton.com`. Claims about those pages come from search-engine extracts of the pages, not from a full read. They are marked **[extract]**. GitHub was reachable, so Ableton's own `Ableton/maxdevtools` repository was cloned and read directly. Those claims are marked **[read]**.
- **Confidence:** High = a primary source states it directly. Medium = a primary source implies it, or only forum posts state it. Low/UNCONFIRMED = inference or nothing found.

## TL;DR

| # | Question | Answer | Confidence |
|---|----------|--------|------------|
| 1 | Can an M4L MIDI effect send MPE to Wavetable on the same track in Live 11? | Yes, if the device's `is_mpe` ("Patch supports MPE") flag is on. Without it, Live collapses MPE onto one voice. | High (flag exists and is meant for this) / Medium (end-to-end Wavetable behaviour) |
| 1 | Bundled Max in Live 11 | Max 8 (8.5.4 from 11.3.2, 8.5.6 from 11.3.20, 8.5.8 from 11.3.25) | High |
| 1 | Ableton MPE M4L devices in Live 11 | MPE Control and Expression Control (Core Library) | High |
| 2 | `js` language level | JavaScript 1.8.5 (SpiderMonkey), about ES5. No ES6. | High |
| 2 | `v8` / `v8ui` | New in Max 9 (ES6+). Max 9 is not supported in Live 11, so `v8` is not available. | High |
| 2 | `js` thread | Always low priority (deferred). Immediate mode deprecated since Max 6. | High |
| 2 | `node.script` | Separate process over IPC, asynchronous. Not suitable for per-note realtime work. | High |
| 3 | `.amxd` format | Chunked binary: `ampf` (device type), `meta`, `ptch` (patcher JSON, or `mx@c` blob if frozen) | High (read from Ableton code) |
| 3 | Generate `.amxd` from text | Feasible (community tools do it), but unofficial. Safer to keep the JSON in git and save once from Max. | Medium |
| 3 | Git practice | Commit the unfrozen device plus separate `.js`/`.maxpat` files; never commit frozen devices (Ableton's advice). | High |
| 4 | Plain Max objects instead of `js` | Yes; the maths is simple enough for `expr`/`mpeformat`, and that keeps it in the scheduler thread. | Medium |
| 5 | State persistence | `live.*` objects or `pattr` with Parameter Mode; `js` has `parameter_enable` plus `getvalueof`/`setvalueof`. | High (docs) / Medium (js blob path, crash reports) |

## 1. MPE output from a Max for Live MIDI effect in Live 11

### The `is_mpe` flag ("Patch supports MPE")

- Live 11 added a Max for Live category to the Patcher Inspector with an `@is_mpe` attribute. Cycling '74: "Setting the @is_mpe attribute enables your device to modify and generate MPE data." It points to the `mpeparse`, `mpeformat` and `polymidiin` help files. **[extract]** https://cycling74.com/articles/what's-new-in-live-11-part-1
- Live 11 release notes describe the same thing as "Patch supports MPE", "to enable MPE for devices that make use of MPE features". **[extract]** https://www.ableton.com/en/release-notes/live-11/
- Ableton's Max for Live production checklist includes "MPE: MIDI devices support MPE (`is_mpe` is set to active)". **[read]** https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md
- The flag is stored in the patcher JSON inside the `.amxd` as `"is_mpe": 0|1`, next to a newer `"external_mpe_tuning_enabled"` key. **[read]** Seen in Ableton's test device `maxdiff/tests/test_files/Test.amxd` (saved by Max 9.1.4), https://github.com/Ableton/maxdevtools/tree/main/maxdiff/tests/test_files. `external_mpe_tuning_enabled` probably belongs to Live 12's tuning system. **UNCONFIRMED**, and it is not relevant to Live 11.
- **Without the flag:** forum users report that `[midiin]` collapses MPE input into a single voice, and that a device "will block and re-interpret MPE messages if MPE is not being activated". **[extract, forum and blog, Medium]** https://cycling74.com/forums/mpe-m4l-and-ableton-11-who's-to-blame! and https://blog.abletondrummer.com/how-to-enable-mpe-for-max-for-live-devices/

**So:** yes, the device needs an option enabled to send MPE: set `is_mpe` = 1 in the Patcher Inspector (or in the JSON).

### Building the MPE stream: `mpeformat` / `mpeparse`

- `mpeformat` "prepare[s] data in the form of a Multidimensional Polyphonic Expression (MPE) MIDI message". It takes `midiformat`-style input per channel and outputs raw MIDI for `midiout`. Its attributes are `chanrange`, `masterchan` (default 1) and `zone` (default 1). By default it creates one Zone Master Channel input and 15 member-channel inputs. **[extract]** https://docs.cycling74.com/reference/mpeformat and https://docs.cycling74.com/max8/refpages/mpeformat
- `mpeparse` splits raw MPE bytes into message types. Its `hires` attribute sets pitch bend format: 0 = 7-bit (default), 1 = float -1..1, 2 = 14-bit integer -8192..8191. **[extract]** https://docs.cycling74.com/reference/mpeparse/
- `mpeconfig` also exists, for MPE Configuration Messages (RPN 6). **[extract]** https://docs.cycling74.com/legacy/max8/refpages/mpeconfig
- A plain `[midiout]` fed raw bytes (note-on `0x9n`, pitch bend `0xEn`) for channels 2 to 16 should also work, because `midiout` passes whatever bytes it receives. This is inference. **[Medium]**

### Known pitfalls (forum reports)

- One user's MIDI effect "was sending all midi on channel 1" and was silent until it forced channel 2. Channel 1 is the MPE Lower Zone master channel, so member notes should use 2 to 16, as The Annoying Piano already plans. **[extract, forum, Medium]** https://cycling74.com/forums/midi-in-to-mpe-out-note-working-in-abelton-11
- Another user found that a chain through `mpeparse` and `midiformat` truncated pitch bend to semitones. The suggested fix was to use 14-bit (`hires 2`) or MPE-aware objects throughout. **[extract, forum, Medium]** https://cycling74.com/forums/mpe-confusion-max-for-live
- One report says `mpeformat` into `midiout` was not interpreted as MPE by Live. It is unclear whether `is_mpe` was set. **[extract, forum, Low]** https://cycling74.com/forums/mpe-to-ableton
- **Evidence that the path works:** Ableton's own free Live 11 device "Envelope to MPE" follows an audio clip's pitch and amplitude and sends them as MPE to an MPE instrument such as Wavetable. **[extract]** https://www.ableton.com/en/blog/live-11-and-max-for-live-get-a-glimpse-of-whats-possible-with-some-free-devices/

### Why the VST3 route failed

- Ableton: "Live merges all MIDI channels to one channel when being routed internally from track to track", so a plug-in's MIDI output (reached via another track's "MIDI From") loses its channels. **[extract]** https://help.ableton.com/hc/en-us/articles/209070189-Accessing-the-MIDI-output-of-a-VST-plug-in
- This matches the observed failure. An M4L MIDI effect on the same track avoids track-to-track routing entirely. **[High]**

### Wavetable and MPE in Live 11

- Wavetable is MPE-capable in Live 11, along with Sampler and (per later sources) Simpler and others. **[extract]** https://help.ableton.com/hc/en-us/articles/360019144999 and https://www.soundonsound.com/reviews/ableton-live-11
- Per-note pitch reaches Wavetable as the "Note PB" modulation source, shown in its MIDI/MPE matrix tab. Per-note pitch is applied to the note's pitch, and Note PB can also be routed to other targets. **[extract, Medium]** https://www.soundonsound.com/reviews/ableton-live-11 and https://morph.sensel.com/pages/live11mpe
- **No instrument toggle needed for native devices:** MPE-capable native instruments show an "MPE" label in their title bar. That is a status label, not a toggle. **[extract, Medium]** https://www.soundonsound.com/reviews/ableton-live-11
- The Preferences > Link/Tempo/MIDI "MPE" switch applies to hardware controller inputs, not to devices on a track, so it should not matter for M4L output. **[extract, Medium]** https://www.ableton.com/en/live-manual/11/editing-mpe/
- Ableton: "The Pitch Bend range of your MPE controller and the MPE-capable instrument need to match for correct tracking." **[extract]** https://help.ableton.com/hc/en-us/articles/360019144999
- **UNCONFIRMED:** that Live 11 treats per-note pitch bend as a fixed ±48 semitones (the MPE spec default). No source states Live's internal per-note range or a Wavetable setting for it. **Test this first:** send a known bend (for example +1 semitone = 8192/48 ≈ 171 steps above centre) and check the result by ear or tuner.

### Bundled Max versions in Live 11

- Live 11 ships with Max 8. **[extract]** https://help.ableton.com/hc/en-us/articles/209772305-Recommended-Max-versions
- Release-note entries:
  - 11.3.2: Max 8.5.4 **[extract]** https://www.ableton.com/en/release-notes/live-11/ and https://help.ableton.com/hc/en-us/articles/360019140859
  - 11.3.10 beta: 8.5.5 **[extract]**
  - 11.3.20: 8.5.6 **[extract]**
  - 11.3.25: 8.5.8 **[extract]**
- A forum log from Live 11.2.11 shows Max 8.3.3. **[extract, forum]** https://forum.ableton.com/viewtopic.php?p=1810844
- **UNCONFIRMED:** the versions bundled with Live 11.0 and 11.1 (probably 8.1.x and 8.2.x).

### Ableton's own MPE M4L devices in Live 11

- **MPE Control** and **Expression Control** MIDI effects are in the Core Library as of Live 11 and are designed for MPE. MPE Control shapes incoming MPE data and maps it to non-MPE instruments. **[extract]** https://help.ableton.com/hc/en-us/articles/360019144999 and https://help.ableton.com/hc/en-us/articles/360019145319-Max-for-Live-Devices-in-Live-11-Intro-and-Standard
- Opening MPE Control in the Max editor is the best working reference for an MPE-aware MIDI effect patch. This is a suggestion, not something checked.
- I found no `live.mpe` object. MPE handling uses `mpeparse`, `mpeformat`, `mpeconfig` and `polymidiin`. **[Medium]**

## 2. JavaScript in Max 8 / Live 11

- **`js` (Max 8):** "JavaScript version 1.8.5", which the Max tutorial calls a Mozilla-specific superset of ECMAScript 5. There are no browser extensions (no `setTimeout`; use `Task`). Do not rely on `let`, arrow functions, classes, template literals, `Map` or `Promise`. **[extract]** https://docs.cycling74.com/max8/vignettes/jsintro and https://docs.cycling74.com/userguide/javascript/
- **`v8` / `v8ui`:** new in Max 9.0.0 (2024), using the V8 engine with ES6+. `js`/`jsui` stay on the old engine for compatibility. **[extract]** https://cycling74.com/releases/max/9.0.0
- **Max 9 is not supported in Live 11:** Ableton says Max 9 is supported as of Live 12 and is "not compatible with Live 11 or earlier". **[extract]** https://help.ableton.com/hc/en-us/articles/209772305-Recommended-Max-versions
- Cycling '74 says the same, and that devices for Live 11 should be authored in Max 8. **[extract]** https://support.cycling74.com/hc/en-us/articles/360051136633-Upgrading-to-Max-9
- **External Max:** Live can use an external Max via Preferences > File Folder > Max Application. **[extract]** https://help.ableton.com/hc/en-us/articles/209070309. For Live 11, only an external Max 8 is supported, so this does **not** unlock `v8`. **[High]**
- **Thread:** "By default, the js object executes all Javascript code in the low-priority thread", and if it finds itself in the high-priority thread it defers. "Immediate" mode has been deprecated since Max 6.0. MIDI input and `metro` are high-priority (scheduler) events. **[extract]** https://docs.cycling74.com/max8/vignettes/jsthreading and https://docs.cycling74.com/userguide/scheduler/
  - **Consequence:** each note through `js` waits in the low-priority queue. That adds jitter and latency, which gets worse when the UI is busy, and the delay is not latency-compensated. For a "nearest semitone plus bend" calculation this is probably acceptable for live play but sloppy for tight timing. Keeping the note path in plain objects (section 4) avoids it. **[Medium]**
- **`node.script`:** runs in a separate Node process and talks to Max over a Unix domain socket. Messages are asynchronous, with "some non zero latency". It is usable in M4L, but Cycling '74 makes no timing guarantees and forum posts report frozen-path problems. It is **not suitable** for the per-note realtime path. **[extract]** https://docs.cycling74.com/reference/node.script, https://cycling74.com/forums/node-script-and-timing-reliability and https://cycling74.com/forums/amxd-file-can%E2%80%99t-find-node-script-after-freezing

## 3. The `.amxd` file format and source control

- **Format [read]**, from Ableton's `maxdiff/amxd_textconv.py`, https://github.com/Ableton/maxdevtools/blob/main/maxdiff/amxd_textconv.py:
  - The file is a sequence of chunks, each a 4-byte ASCII tag plus a 4-byte little-endian length plus data.
  - `ampf` (4 bytes) holds the device type: `mmmm` MIDI effect, `aaaa` audio effect, `iiii` instrument (`nagg`/`natt` are Live 12 MIDI Tools).
  - `meta` (4 bytes; observed `01 00 00 00`).
  - `ptch` holds the patcher JSON (`{"patcher": {...}}`, optionally NUL-terminated). If frozen, it starts with `mx@c` and is a container of embedded files.
  - `ciph` marks an encrypted device.
- **Verified:** parsing Ableton's sample `Test.amxd` gave exactly `ampf 4 'mmmm'`, `meta 4`, `ptch 58823 '{"patcher":...'`, with the chunk lengths summing to the file size. The patcher JSON also carries `project.amxdtype` = 1835887981, which is `"mmmm"` as a 32-bit integer.
- **Generating outside Max:** this works in practice. Community tools write `.amxd` from code:
  - `py2max` claims byte-compatible output. https://github.com/shakfu/py2max
  - `js2max` (targets Max 9 `v8`) was reverse-engineered from maxdevtools. https://github.com/ktamas77/js2max/
  - Neither is official; Ableton publishes no spec. **[Medium]**
  - **Recommendation:** generate the JSON, wrap it in the three chunks, then open and re-save it once in Max 8 to normalise it. Keep the JSON in git.
- **Git practice:**
  - Ableton recommends keeping "the original versions (not the frozen copies) of your plugins and their dependencies" in git. **[read]** https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md
  - maxdiff: "We recommend never to commit frozen devices to a git repository, instead to include the dependencies as separate files." It also gives a git `textconv` diff driver for `.amxd`/`.maxpat`. **[read]** https://github.com/Ableton/maxdevtools/blob/main/maxdiff/README.md
  - Ableton's JS style: `"use strict";`, two-space indent, trailing newline. **[read]** https://github.com/Ableton/maxdevtools/blob/main/patch-code-standard/patch-code-standard.md
  - **Workflow:** commit the unfrozen `.amxd` plus `.js` sources. Freeze only to produce a release copy, and do not commit that copy.
- **Freezing** embeds dependencies, including JavaScript, into the device. Freezing takes effect on save. Embedded files take priority over same-named files in the search path when a frozen device is opened. **[extract]** https://docs.cycling74.com/userguide/m4l/live_freezing/
- **How `js` files are found (unfrozen):** each device acts as a Max project. Max looks in the device's project/folder first, then the Max search path. When an unfrozen device is opened or saved, Max may unpack dependencies into `~/Documents/Max 8/Max for Live Devices/<Device> Project/`.
  - Forum reports say lookup is not always consistent, and repeated freeze/unfreeze creates duplicate copies. **[extract, docs plus forum, Medium]** https://cycling74.com/forums/max-for-live-freezing-filepaths-and-unclear-behavior, https://cycling74.com/forums/ensure-js-files-are-in-search-path and https://docs.cycling74.com/max8/refpages/project
  - The Ableton guideline confirms the unfreeze folder. **[read]**
  - **Practical rule:** during development, keep the `.js` next to the `.amxd`, or add the repo folder to Options > File Preferences. Freeze for distribution.

## 4. Plain Max objects (or gen) instead of `js`

- The per-note maths is a small calculation: nearest semitone = round(f), bend = (f - semi) / 48 × 8192 + 8192, plus channel allocation over 2 to 16. Plain objects can do it: `expr`, `round`, `mpeformat`/`midiformat`, `zl`/`coll`/`table` for the tuning map, and a small channel rotator. All of these run in the scheduler thread with MIDI, unlike `js`. **[Medium; threading per** https://docs.cycling74.com/userguide/scheduler/ **]**
- **Event-domain `gen`** (codebox "genexpr") is described as an extended `expr`. **UNCONFIRMED:** whether it runs in the high-priority thread. https://cycling74.com/forums/gen-vs-javascript-vs-max
- **Pragmatic split:** `js` only for non-realtime work (building or loading the tuning table into a `coll`/`buffer`/`dict`). The note path uses objects.

## 5. Parameter persistence

- `live.*` UI objects are Live parameters by default. Their values are stored with the Live Set and can be automated when Parameter Visibility is "Automated and Stored". Ordinary Max UI objects and `pattr` can become parameters via Parameter Mode (`parameter_enable`). Types are Int, Float, Enum and Blob; Blob is stored but not automatable. **[extract]** https://docs.cycling74.com/userguide/m4l/live_parameters and https://docs.cycling74.com/userguide/parameter_mode/. Ableton's guide says the same. **[read]**
- **`js` state:**
  - `js` has a `parameter_enable` attribute. **[extract]** https://docs.cycling74.com/reference/js
  - A `js` can expose `getvalueof()`/`setvalueof()` so that `pattr` (in Parameter Mode, type Blob) can store it. One forum user reports Max crashing with js plus pattr as Blob, and suggests a `dict` or a serialised string instead. **[extract, forum, Medium]** https://cycling74.com/forums/how-to-save-javascript-object-to-max-for-live-preset-using-pattr
- **Recommendation:** keep user-facing settings in `live.tab`/`live.dial`/`live.numbox` and feed their values into the `js`. The `js` should hold no state that is not derivable from those.

## Things to verify by hand in Live 11

1. A minimal device (`is_mpe` on, `midiin` → bytes on channels 2 to 16 with a pitch bend before each note-on → `midiout`) into Wavetable: are the per-note bends independent?
2. The per-note bend scale Live/Wavetable applies (is it ±48?).
3. With `is_mpe` off, confirm that the bends collapse. This confirms the flag is what matters.
4. Exact bundled Max version: Live > Preferences > File Folder, or `max version` in the Max console.
