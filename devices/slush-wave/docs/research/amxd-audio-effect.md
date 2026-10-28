# Packing an audio effect `.amxd` with `tools/amxd.mjs`

Ticket: [#4 What does tools/amxd.mjs need to pack an audio effect?](https://github.com/jimmyNicholas/audio-plugins/issues/4)

## Answer in short

- The container is the same for every device type. Only the 4 bytes of the `ampf` chunk change: `aaaa` for an audio effect instead of `mmmm`. `meta` and `ptch` stay as they are.
- The patcher JSON carries the type a second time, as `patcher.project.amxdtype`. For an audio effect it is `1633771873` (`"aaaa"` read as a big-endian 32-bit integer), not the `1835887981` (`"mmmm"`) that TAP's `.maxpat` files carry. So a device type option in the tool is only half the job: the `.maxpat` must agree, or the tool must check or set it.
- The patch needs `plugin~` (audio in from Live) wired to `plugout~` (audio back to Live), left to left and right to right. Ableton's own audio effect sample has exactly that pair, each with 2 inlets and 2 signal outlets and no arguments.
- `is_mpe` is a MIDI matter and can be left at 0. `latency` (Defined Latency) starts to matter for an audio effect that delays its signal.

**Recommendation:** give `pack` (and the CLI) a device type option that defaults to MIDI effect, and have `pack` refuse a patcher whose `project.amxdtype` does not match the chosen type. A spike audio effect patch is then `plugin~` to `plugout~` plus `live.thisdevice`, with `amxdtype` 1633771873.

## What the tool does today

Read from `tools/amxd.mjs` and `devices/tap/tests/amxd.test.mjs` in this repo.

- `pack(patcherText, deviceType = MIDI_EFFECT)` writes three chunks, each a 4-byte ASCII tag, a 4-byte little-endian length and the data:
  - `ampf`: the device type, 4 ASCII bytes. `MIDI_EFFECT = "mmmm"` is the only exported constant.
  - `meta`: 4 zero bytes.
  - `ptch`: the patcher JSON, a trailing newline added if missing, then one NUL byte.
- `pack` already takes the device type as a second argument; nothing calls it with anything but the default. The CLI (`node tools/amxd.mjs pack <in> <out>`) has no way to pass it.
- `pack` does not look inside the JSON beyond `JSON.parse`, so it never checks `project.amxdtype`.
- The tests check the chunk order, the `mmmm` tag, the 4-byte `meta`, the trailing NUL, a round trip, broken JSON, a frozen device, and that `TAP.amxd` and `spike/TAP MPE Spike.amxd` match `pack` of their `.maxpat`.
- `devices/tap/TAP.amxd` starts `ampf 04000000 mmmm meta 04000000 00000000 ptch ...`, and both TAP `.maxpat` files have `"amxdtype": 1835887981` in their `project` block (checked with `od` and `grep`).
- `devices/tap/docs/HANDOFF.md`: "The generated `.amxd` loaded straight away, no re-save in Max." So this three-chunk layout with a zero `meta` is known to load in Live 11 as a MIDI effect.

## The device type code

- **`aaaa` is the audio effect code. [read]** Ableton's `maxdiff/amxd_textconv.py` maps `ampf` values to names: `"aaaa": "Audio Effect Device"`, `"mmmm": "MIDI Effect Device"`, `"iiii": "Instrument Device"`, `"nagg": "MIDI Tool Generator"`, `"natt": "MIDI Tool Transformation"`. It rejects an `ampf` whose length is not 4. https://github.com/Ableton/maxdevtools/blob/main/maxdiff/amxd_textconv.py (commit 5b39dec, Sept 2026)
- **Verified on real files.** Ableton's test devices, parsed chunk by chunk with the same reader as `readChunks`:

  | File | `ampf` | `meta` bytes | Next chunk |
  | --- | --- | --- | --- |
  | `Test.amxd` | `mmmm` | `01 00 00 00` | `ptch`, plain JSON |
  | `FrozenTest.amxd` | `mmmm` | `07 00 00 00` | `ptch`, `mx@c` (frozen) |
  | `WithGarbage.amxd` | `aaaa` | `00 00 00 07` | `ptch`, `mx@c` (frozen) |
  | `EncryptedTest.amxd` | `aaaa` | `00 00 00 00` | `ciph`, then `ptch` |

  Ableton's own expected output for the two `aaaa` files starts "Audio Effect Device" (`maxdiff/tests/test_baselines/WithGarbage.amxd.txt`, `EncryptedTest.amxd.txt`). https://github.com/Ableton/maxdevtools/tree/main/maxdiff/tests
- **Same chunk layout across types. [read]** Audio and MIDI samples use the same `ampf`, `meta`, `ptch` order and the same 4-byte `ampf` and `meta`. Nothing in `amxd_textconv.py` treats `ptch` differently by device type.

## The `meta` chunk

- **Its meaning is not documented. [read]** `amxd_textconv.py`'s `handle_meta` returns an empty string; Ableton's tool ignores the contents.
- Observed values differ (0, 1, 7, and `00 00 00 07`) and do not follow the device type: one audio effect has all zeros, the other does not. **UNCONFIRMED** what the bytes mean; a flags field or a save counter are both plausible guesses.
- TAP loads with all zeros, so there is no evidence an audio effect needs anything else. **Keep `meta` at zero.** If Live refuses the spike, `meta` is the second thing to try (after `amxdtype`), with `01 00 00 00` as in `Test.amxd`.

## `project.amxdtype` inside the patcher JSON

- **The audio sample carries `1633771873`. [read]** `grep` of `WithGarbage.amxd` finds `"amxdtype" : 1633771873` in its device patcher's `project` block (a second, `0`, sits in the trailing junk that gives the file its name). `Test.amxd` and Ableton's baseline for it have `amxdtype: 1835887981`.
- `1633771873` is `0x61616161`, `"aaaa"` read big-endian; `1835887981` is `0x6D6D6D6D`, `"mmmm"`; `1768515945` would be `"iiii"` (checked in Node).
- **py2max does the same. [read, community]** Its `m4l.py` sets `project.amxdtype` from the device type ("The value is the FOURCC tag reinterpreted as a big-endian uint32") and adds the whole `project` block when it is missing, saying that without it Max reports "a project without a name is like a day without sunshine. fatal." https://github.com/shakfu/py2max (commit f0e2987, `py2max/m4l.py`)
- **What happens on a mismatch is UNCONFIRMED.** No source says whether Live goes by `ampf` or by `amxdtype`, or whether a mismatch fails, loads as the wrong type, or is fixed on the next save. Avoid the question by keeping them equal. The tool can enforce that cheaply: read `patcher.project.amxdtype` after `JSON.parse` and throw if it is not the chosen type's integer.
- The rest of the `project` block in TAP's `.maxpat` (`version`, `contents`, `devpath` and so on) matches the audio sample's apart from `readonly` (1 there, because it was frozen) and `includepackages` (absent there). Copy TAP's block and change only `amxdtype`.

## Patcher contents for an audio effect

- **`plugin~` and `plugout~`, wired straight through. [read]** In `WithGarbage.amxd` (a "Livesync Leader" audio effect) the device patcher has `plugin~` (`obj-1`) and `plugout~` (`obj-2`), each `"numinlets": 2, "numoutlets": 2, "outlettype": ["signal", "signal"]`, with patchlines `obj-1` outlet 0 to `obj-2` inlet 0 and outlet 1 to inlet 1. The device does no audio work; the pair just passes audio through.
- **Box shape. [read]** Ableton's `maxdiff/known_objects.py` lists both with exactly those inlet and outlet counts and types and plain text `plugin~` / `plugout~` (no arguments), so a generated box should copy that.
- Ableton's code standard treats `plugin~` as "Audio coming from Live", basic enough not to need a comment. https://github.com/Ableton/maxdevtools/blob/main/patch-code-standard/patch-code-standard.md
- py2max's README builds its audio effect example as `plugin~` to a gain to `plugout~`. [read, community]
- **What happens with no `plugout~`.** No primary source was reachable to confirm this (see Sources not reached). The common understanding is that an audio effect without `plugout~` outputs silence and one without `plugin~` hears nothing. **UNCONFIRMED.** A spike should check it by leaving the pair in and only then experimenting.
- **MIDI objects.** `midiin` / `midiout` are not needed in an audio effect. **UNCONFIRMED** whether an audio effect may still use `midiin` (it would see MIDI only on a MIDI track before the instrument, and audio effects usually sit after it).

## Anything else different

- **`is_mpe`.** A MIDI flag (see `devices/tap/docs/RESEARCH.md`, section 1). Set it to 0 in an audio effect.
- **`latency`.** TAP's patcher has `"latency": 0`. Ableton's production guidelines: "Max for Live allows setting **Defined Latency** in the patch Inspector, Live compensates for this latency on playback", and their checklist has "Latency: The device plays in sync with the rest of the Live Set." [read] https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md. It is 0 for a pass-through spike; it needs a real value once the effect adds delay (a lookahead or an FFT window, for example).
- **Device width, panel, `live.thisdevice`, parameters, freezing, `js` lookup.** Nothing found that differs by device type. Everything in `RESEARCH.md` section 3 (keep the unfrozen device in git, freeze only for release) applies unchanged.
- **Frozen and encrypted devices.** Both audio samples are frozen or encrypted; `unpack` already refuses `mx@c`. It does not know `ciph` (it would look for `ptch` and then fail to read it as text), which matters only if someone feeds it an encrypted device.
- **py2max writes a different container.** It wraps even unfrozen patchers in an `mx@c` block with a trailer of extra chunks, and uses big-endian sizes inside that. Ableton's own `Test.amxd` and TAP use the plain little-endian three-chunk form, and TAP loads, so there is no reason to follow py2max here.

## What this means for the tool

1. Export `AUDIO_EFFECT = "aaaa"` (and `INSTRUMENT = "iiii"` costs nothing) next to `MIDI_EFFECT`.
2. Add a CLI option, for example `pack <in.maxpat> <out.amxd> [--type audio|midi|instrument]`, defaulting to MIDI so the TAP build script and its byte-for-byte tests do not change.
3. In `pack`, after `JSON.parse`, compare `patcher.project.amxdtype` with the type's integer (`Buffer.from(type).readUInt32BE(0)`) and throw on a mismatch, or when `project` is missing. Checking is better than rewriting: rewriting would make the `.maxpat` in git disagree with the `.amxd`.
4. Tests: an `aaaa` pack, the mismatch error, and an in-sync test for the spike's `.amxd` like TAP's.
5. `unpack` needs no change.

## What a spike patch must contain

- `project.amxdtype`: `1633771873`, rest of the `project` block copied from `TAP.maxpat`.
- `is_mpe`: 0; `latency`: 0.
- `plugin~` (2 in, 2 signal out) wired left to left and right to right into `plugout~` (2 in, 2 signal out).
- `live.thisdevice` if the patch needs a load bang, as in TAP.
- Packed with `ampf` `aaaa`, `meta` zero.
- **Check by hand in Live 11:** it appears and drops as an audio effect, audio passes unchanged, and it saves and reloads with the set. If it does not load, change `meta` to `01 00 00 00`, then try opening and re-saving it once in Max and diff the result with `unpack`.

## Sources

- This repo: `tools/amxd.mjs`, `devices/tap/tests/amxd.test.mjs`, `devices/tap/TAP.amxd`, `devices/tap/TAP.maxpat`, `devices/tap/spike/TAP MPE Spike.maxpat`, `devices/tap/docs/RESEARCH.md`, `devices/tap/docs/HANDOFF.md`.
- Ableton maxdevtools, commit 5b39dec: `maxdiff/amxd_textconv.py`, `maxdiff/known_objects.py`, `maxdiff/tests/test_files/{Test,FrozenTest,WithGarbage,EncryptedTest}.amxd`, `maxdiff/tests/test_baselines/`, `m4l-production-guidelines/m4l-production-guidelines.md`, `patch-code-standard/patch-code-standard.md`. https://github.com/Ableton/maxdevtools
- py2max, commit f0e2987: `py2max/m4l.py`, `README.md`. https://github.com/shakfu/py2max (community, not official)

## Sources not reached

- The Cycling '74 reference pages for `plugin~` and `plugout~` (https://docs.cycling74.com/reference/plugin~ and https://docs.cycling74.com/reference/plugout~) were blocked by this session's network settings. They would settle the default channel arguments and what a device without `plugout~` does. Allow `docs.cycling74.com` and re-read them to firm up the two UNCONFIRMED points above.
