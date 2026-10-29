# Slush Pitch Layer Audition: build spec

A throwaway Max for Live audio effect that runs every candidate way of building `Slush Pitch Layer` side by side, with switches to flip between them on the same audio. The user loads it in Live 11, listens, and reports back, and those notes settle the open ticket [How is Slush Pitch Layer built?](https://github.com/jimmyNicholas/audio-plugins/issues/10).

This is **not** the map's spike ([Build and check the Slush Wave spike in Live](https://github.com/jimmyNicholas/audio-plugins/issues/8)) and **not** the real device. It is never released, and nothing in it is reused without review. Prefer simple and obviously right over efficient or tidy.

Terms (pitch layer, detuned doubles, interval layer, sides path) are defined in [CONTEXT.md](../../CONTEXT.md). Background: [max-building-blocks.md](../../docs/research/max-building-blocks.md) section 1, [amxd-audio-effect.md](../../docs/research/amxd-audio-effect.md).

## Who does what

- **Builder (an agent, no Max or Live):** writes the patch as text, packs it, adds the tool change and tests, and writes the README walkthrough. Cannot run it.
- **User (Live 11 Suite, bundled Max 8, Intel Mac, macOS 13):** loads it, runs the checks, reports back.

So the patch must be easy to debug from a Max Console error message: simple objects, clear subpatcher names, comments on anything non-obvious.

## Decisions the audition must let the user hear

| Ref | Decision | Switch on the panel |
|---|---|---|
| Q1 | How the detuned doubles are made | Doubles engine: Delay line / Pitchshift |
| Q2 | How the interval layer is made, and at what quality | Interval engine: Pitchshift / Delay line; Quality; Window |
| Q5 | Where each double comes from | Doubles source: Per channel / Mono sum |
| Q6 | Interval layer in mono or per channel | Interval stereo: Mono centre / Per channel |
| Q7 | Whether the doubles drift | Drift: Off / On, plus depth and rate |
| Q8 | Latency, alignment and CPU | Readouts, plus mute on unused versions |

Q3 (what the device outputs) and Q4 (which intervals) are settled by talking, not listening. The audition just assumes the recommended answers: the input passes at unity, and the interval menu is octave down and fifth down.

## Hard constraints

- **No `gen~`.** The Max licence on hand can't edit it. Plain MSP and `live.*` objects only.
- **No `js`.** It isn't needed, and it avoids the ES5 check and file lookup problems.
- **No third-party externals or packages.**
- **Audio effect device type:** `ampf` `aaaa`, `project.amxdtype` `1633771873` (see "Tool change" below).
- **Unfrozen**, like TAP. Every subpatcher is embedded (`p` boxes), with no abstraction files, so nothing depends on Max's search path.
- Australian spelling and no em dashes in all text, comments and the README.

## Files

All under `devices/slush-wave/prototype/pitch-layer-audition/`:

| File | What |
|---|---|
| `SPEC.md` | This file. |
| `build-patch.mjs` | Recommended: a Node script that generates the `.maxpat` from a compact description (boxes, lines, presentation rects), so the JSON is consistent and reviewable. It is `.mjs`, so CI's ES5 check on `*.js` doesn't apply. Hand-writing the JSON is acceptable if the builder prefers. |
| `Slush Pitch Layer Audition.maxpat` | The patcher JSON, source of truth. |
| `Slush Pitch Layer Audition.amxd` | `node tools/amxd.mjs pack --type audio ...` of the `.maxpat`. |
| `README.md` | The user's walkthrough and listening checklist, in the style of `devices/tap/spike/README.md`. Content in "Listening checklist" below. |

Plus the tool change in `tools/amxd.mjs` and tests (see below).

## Signal flow

```
plugin~ L,R
  ├─ Dry ─────────────── delay~ (Lmax) ─────────────────────────────┐
  ├─ Doubles ─┬─ [p doubles-delayline]  (muted unless selected)     ├─ sum ─ plugout~ L,R
  │           └─ [p doubles-pitchshift] (muted unless selected)     │
  └─ Interval ┬─ [p interval-pitchshift] (muted unless selected)    │
              └─ [p interval-delayline]  (muted unless selected)  ──┘
```

- Every version is its own `p` subpatcher with a `mute~` in front, so only the selected engine for each layer uses CPU (CPU readings in Live then reflect one version at a time). A layer whose level is at its minimum (-inf) mutes both of its versions.
- Selection after the subpatchers is a crossfade or `selector~`. A click on switching is acceptable in an audition, but a short (about 20 ms) `line~` crossfade is nicer if cheap.
- Output: `L = dry_L + doubles_L * doublesLevel * trim + interval_L * intervalLevel * trim`, the same for R. No limiter or clip.

### Latency and alignment

- **Defined Latency** (`"latency"` in the patcher JSON) is a constant `Lmax = 8192` samples. It never changes at runtime.
- The **dry** path is delayed by exactly `Lmax` with `delay~ 16384 8192`.
- Each **pitchshift** version is followed by its own `delay~` set to `Lmax - L`, where `L` is that instance's measured latency from `getlatency`. Measure on load (deferred, after `live.thisdevice` bangs) and again whenever Quality changes. If `L > Lmax`, show a warning in the panel and print it to the Max Console.
- The **delay line** versions are delayed by `Lmax` as well. Their own small sweeping delay (the base delay plus the window) is part of their sound and is **not** compensated.
- Print every measured latency to the Max Console as `audition: pitchshift <layer> <quality> latency <n> samples`, and show it on the panel.

### Delay line shifter (used by both layers' delay line versions)

One shifter = a `tapin~`/`tapout~` pair with two taps. Per channel per voice:

- Pitch ratio `p = 2^(cents / 1200)` (for the interval, cents = -1200 for an octave down, -700 for a fifth down).
- Window `W` ms. Ramp frequency `f = (1 - p) * 1000 / W` Hz. This is negative for upward shifts, so the `phasor~` runs backwards, which is correct.
- `phasor~ f` gives phase `φ`. The second tap uses `φ2 = (φ + 0.5) mod 1` (`+~ 0.5` then `%~ 1.`).
- Tap delay (ms) = `base + φ * W`, with `base = 3` ms (above one signal vector at 64 samples and 44.1 kHz or more; the `tapout~` minimum).
- Tap gain = Hann window `0.5 - 0.5 * cos~(φ)` (`cos~` takes its input in cycles). The two offset windows sum to 1.
- Out = tap1 * w1 + tap2 * w2.
- `tapin~` max delay: 200 ms.
- Compute `f` at signal rate with `expr~ (1. - pow(2., $v1 / 1200.)) * 1000. / $f2`, so drift (a signal added to cents) works smoothly. `$v1` is the cents signal and `$f2` is the window in ms.

Doubles window: fixed 30 ms. Interval window: the **Window** dial.

### `pitchshift~` versions

- One mono `pitchshift~` per channel per voice (don't rely on a multichannel argument). Ratio via the `pitchshift` attribute (a factor, `2^(cents / 1200)`), `quality` from the Quality menu.
- Drift on a pitchshift double: update the factor from the drift signal by `snapshot~` about every 20 ms. Coarse steps are fine.
- Check the exact inlets, outlets and attribute names against the Max 8 reference (`pitchshift~` help and refpage). The research could only read search extracts, so treat names here as "verify, then use".

### Stereo routing

- **Doubles, per channel:** L into the up shifter (+cents), output left. R into the down shifter (-cents), output right.
- **Doubles, mono sum:** `(L + R) * 0.5` into both shifters. Up goes hard left, down goes hard right.
- **Interval, mono centre:** `(L + R) * 0.5` into one shifter, output equally to L and R (no extra pan law gain).
- **Interval, per channel:** L into a shifter, output left. R into a second shifter, output right.

### Drift (doubles only)

`rand~ <rate>` (smoothed random, -1 to 1) times the depth in cents, added to +cents for the up double and subtracted from -cents for the down double (each double gets its own `rand~` so they wander independently). Off means the drift signal is 0.

## Controls

All are `live.*` objects in presentation mode, saved with the Set (parameter visibility: Stored Only is fine; automation isn't needed). Device height is Live's standard 169 px; the width is whatever fits. Group them with `live.comment` labels as below.

**Layers**
- **Layers:** On / Off (`live.text` toggle). Off mutes both layers and leaves the delayed dry signal, for A/B against the plain input at the same timing. Default On.
- **Solo:** Off / Doubles / Interval (`live.tab`). Doubles or Interval mutes the dry signal and the other layer. Default Off.

**Doubles**
- **Engine:** Delay line / Pitchshift (`live.tab`). Default Delay line.
- **Source:** Per channel / Mono sum (`live.tab`). Default Per channel.
- **Cents:** 0 to 30, default 12 (`live.dial`, step 1).
- **Level:** -inf to 0 dB, default -9 dB (`live.dial`, dB units).
- **Drift:** Off / On (`live.text`). Default Off.
- **Drift depth:** 0 to 10 cents, default 3.
- **Drift rate:** 0.05 to 1 Hz, default 0.2.
- **Trim DL / Trim PS:** -12 to +12 dB, default 0, one per engine, for level matching.

**Interval**
- **Engine:** Pitchshift / Delay line (`live.tab`). Default Pitchshift.
- **Quality:** basic / good / better / best (`live.menu`). Pitchshift only. Default good.
- **Window:** 20 to 150 ms, default 60 (`live.dial`). Delay line only.
- **Interval:** Octave down / Fifth down (`live.tab`). Default Octave down.
- **Stereo:** Mono centre / Per channel (`live.tab`). Default Mono centre.
- **Level:** -inf to 0 dB, default -12 dB.
- **Trim PS / Trim DL:** -12 to +12 dB, default 0.

**Readouts** (`live.comment` or `live.numbox` set to display only)
- Measured `pitchshift~` latency for the active doubles and interval instances, in samples and ms.
- Defined Latency: 8192 samples (static text).
- A warning line, empty unless a measured latency exceeds `Lmax`.

## Tool change: audio effects in `tools/amxd.mjs`

As recommended in [amxd-audio-effect.md](../../docs/research/amxd-audio-effect.md):

- Export `AUDIO_EFFECT = "aaaa"` next to `MIDI_EFFECT`.
- In `pack`, after `JSON.parse`, read `patcher.project.amxdtype` and throw if it is missing or not the chosen type's integer (`Buffer.from(type, "ascii").readUInt32BE(0)`).
- The CLI gains `--type midi|audio` (default `midi`), for example `node tools/amxd.mjs pack --type audio <in.maxpat> <out.amxd>`. Existing TAP commands keep working unchanged.
- Tests (next to the existing `amxd` tests, `devices/tap/tests/amxd.test.mjs`, or move the tool's tests beside the tool if that is clearly tidier): `aaaa` tag written; a mismatched or missing `amxdtype` throws for both types; the audition `.amxd` equals `pack` of its `.maxpat` with type audio. `npm test` passes and TAP's existing tests still pass.
- The `project` block is copied from `devices/tap/TAP.maxpat` with only `amxdtype` changed. `is_mpe` 0. `latency` 8192.

## Listening checklist (content for README.md)

Write the README in the TAP spike style: get the files, add the folder to Live's Places, load the device, troubleshooting (Max Console red lines, copy them back), then these checks. Number every step.

**Setup:** an audio track with a slowed loop (pre-slowed: Re-Pitch warp at about 0.8x), and a second test on a vocal or a single instrument. Put the device first on the track. Optionally duplicate the track with no device on it, to check alignment.

1. **Alignment (Q8):** Layers Off, then toggle the device on and off. With the duplicate track, the two should stay in time with no flam. Then Layers On, both levels up: listen for comb filtering or flamming between the input and the layers. Report: clean / flams / combs.
2. **Doubles engine (Q1):** Solo Doubles, Cents 12. Flip the engine, then repeat at Cents 5 and 25. Match level with the trims first. Report: which you prefer, and whether you can tell them apart in the full sound (Solo Off).
3. **Doubles source (Q5):** Solo Off. Flip Per channel and Mono sum. Report: which is wider or nicer, and anything odd.
4. **Drift (Q7):** Doubles on their own, Drift On, depth 3, rate 0.2, then try other settings. Report: Off or On, and settings you liked.
5. **Interval engine and quality (Q2):** Solo Interval, Octave down. Pitchshift at each Quality, then Delay line at Windows 30, 60 and 120. Then Solo Off and judge it under the full sound at Level -12. Repeat for Fifth down. Report: the lowest Quality that sounds fine, whether the delay line's grain is usable or even wanted, and the latency readout for each Quality.
6. **Interval stereo (Q6):** Flip Mono centre and Per channel under the full sound. Report: can you hear a difference, and which you prefer.
7. **CPU (Q8):** Live's CPU meter (top right) with the device off, then on with each interval engine and Quality. Optional: duplicate the track until the meter reaches about 50%, as Ableton's CPU Reporter guidance suggests. Report: the readings and the buffer size (Preferences > Audio).
8. **Overall:** your favourite combination, with all settings written down.

**What to send back:** answers to 1 to 8, the latency readouts, the Live and Max versions, and any Max Console red lines.

## Done when

- `npm test` passes, including the new tool tests.
- The `.amxd` is packed from the committed `.maxpat` with type audio.
- The README walkthrough and checklist exist.
- The builder has re-read the patch against this spec: every switch from the decisions table is present, every pitchshift version's compensation path is wired, and the dry path carries `delay~` at `Lmax`.
- Committed and pushed. The builder notes anything it couldn't confirm (for example `pitchshift~` attribute names) in the README's troubleshooting section, so the user knows where to look first.

## Out of scope

- The real Slush Pitch Layer device, its final panel, and its parameter ranges (those belong to the controls ticket).
- `gen~`, `pfft~`, or any engine not listed here.
- Freezing, presets, Push mapping, automation.
- Mono input handling beyond what falls out naturally (an open map question).
