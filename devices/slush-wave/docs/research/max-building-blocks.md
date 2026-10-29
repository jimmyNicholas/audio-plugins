# Max 8 building blocks for each Slush Wave stage

Ticket: [#3 Which Max 8 building blocks can do each stage in real time?](https://github.com/jimmyNicholas/audio-plugins/issues/3)

Researched 2026-10-28. Target: a Max for Live audio effect in Ableton Live 11 Suite with its bundled Max 8 (8.5.4 from Live 11.3.2, 8.5.6 from 11.3.20, 8.5.8 from 11.3.25; see [TAP's research](../../../tap/docs/RESEARCH.md) and the [Live 11 release notes](https://www.ableton.com/en/release-notes/live-11/)), on an Intel Mac, macOS 13. Stages come from [slushwave-sound.md](slushwave-sound.md).

## How to read this

- **Method caveat:** the sandbox could not fetch `docs.cycling74.com`, `cycling74.com`, `support.cycling74.com` or `help.ableton.com` (DNS and proxy blocked). Claims about those pages come from search-engine extracts of the named page, marked **[extract]**. Ableton's `Ableton/maxdevtools` repository on GitHub was cloned and read directly, marked **[read]**. Arithmetic and design reasoning by the researcher is marked **[inference]**.
- **CPU:** no primary source publishes CPU figures for any of these objects. CPU rankings below are **[inference]** from what each object does (a few multiplies per sample versus an FFT versus a commercial time-stretch engine). Ableton's own advice is to measure in a real Set with its CPU Reporter device (see section 6).
- **Confidence:** High = a primary source states it. Medium = a primary source implies it, or only forum posts state it. Low = inference only.

## Answer in short

| Stage | Best fit in Max 8 | Ships with Max 8? | Latency | CPU (relative, inference) |
|---|---|---|---|---|
| Pitch down, keep time (live) | `gen~` delay-line pitch shifter (as in the `gen~.pitchshift` example) | Yes (`gen~` and its examples) | Small and variable: a few to tens of ms of delay that sweeps; nothing to report as fixed | Low |
| Pitch down, keep time (live), higher quality | `pitchshift~` (or `retune~`, which is for intonation correction) | Yes (MSP, since Max 7) | Reported by the object via `getlatency`; one forum user saw about 4000 samples (about 83 ms at 48 kHz) | High |
| Pitch down, keep time, spectral | `pfft~` phase vocoder | Yes (MSP) | Documented: window size minus hop size (for example 1536 samples = 32 ms for 2048 / overlap 4) | Medium to high |
| True slowdown (pitch and tempo together) | `record~` (loop mode) into `buffer~`, read by `groove~` at a speed below 1, or `gen~` reading a circular buffer | Yes (MSP) | No processing latency, but the output lags the input by a growing amount until it is reset | Low |
| Reverb | `gen~ gigaverb` or `gen~ freeverb` example, `yafr2` abstraction, `cverb~`, or hand-built FDN/Dattorro in `gen~` | Yes, all of them | Zero (feedback delays are not latency) | Low to medium |
| Chorus, wow, flutter, phaser | Modulated `delay` in `gen~` (as in `gen~.flange_chorus`), allpass chain in `gen~` for phaser | Yes | Zero to a few ms of base delay (part of the sound, not reported) | Low |
| Filtering | `svf~`, `biquad~` fed by `filtercoeff~`, `lores~`, or `gen~` filters | Yes (MSP) | Zero | Very low |
| Saturation | `overdrive~`, or `tanh` and friends in `gen~` | Yes (MSP) | Zero | Very low (unless oversampled) |
| Latency report | **Defined Latency** in the Patcher Inspector (`"latency"` in the patcher JSON), in samples | Built in | n/a | n/a |

**Nothing in this list needs a package.** Every object above is part of Max 8's MSP or Gen, and the reverbs and gen~ examples are patches in Max's own examples folder. The catch is not packages but two other things:

1. **Licence for `gen~`.** Running a device that contains `gen~` needs only Live Suite's Max for Live. **Creating or editing** a `gen~` patcher needs a standalone Max (or crossgrade) licence (sources in section 7). Whether the user has one decides whether `gen~` is a free choice or a cost. Writing the gen code as text and packing it with `tools/amxd.mjs` may sidestep the editor, but that is untested and should be checked against the licence terms.
2. **Examples are files outside the device.** `gen~ gigaverb`, `yafr2` and the other examples load from Max's search path. Before distributing, the device must be frozen (or the code copied into the device) so it does not depend on the examples folder being present.

## 1. Pitching down and slowing down live audio

### 1.1 The core constraint: a live stream cannot be slowed forever

- To play input at speed `r < 1`, the read head moves `r` samples per input sample, so it falls behind by `(1 - r)` seconds every second. The gap grows linearly with no limit. **[inference, High]**
- With a buffer of `D` seconds, playback can run for `D / (1 - r)` seconds before the read head would need audio older than the buffer holds. Examples at 0.8x: a 10 s buffer lasts 50 s; a 30 s buffer lasts 150 s. At 0.65x a 30 s buffer lasts about 86 s. **[inference]**
- Memory is cheap: 30 s of stereo 32-bit float at 48 kHz is 30 x 48000 x 2 x 4 bytes, about 11.5 MB. Memory is not the problem; the ever-growing lag (the slowed audio drifts out of time with the Set) is. **[inference]**
- So every bounded approach must do one of these: (a) shift pitch only and keep time (no growing lag), (b) play a captured section slowed and then jump back (reset on a cycle, a bar or Live's transport), or (c) let Live slow the source (clip warping or tempo) before the device. This is the choice ticket #5 makes. **[inference, High]**

### 1.2 `gen~` delay-line pitch shifter (pitch down, keep time)

- `gen~` "turns an embedded Gen patcher into signal processing routines of optimized native machine code", processing per sample. **[extract]** https://docs.cycling74.com/max8/vignettes/gen_overview
- The gen~ `delay` operator: `@feedback 1` by default (adds one sample of minimum delay), `interp` modes `none`, `step`, `linear` (default), `cosine`, `cubic`, `fastcubic`, `spline`, `spline6`; in Max 8 its default maximum delay is one second (the sample rate in samples), and delay and history apply `fixdenorm` by default. **[extract]** https://docs.cycling74.com/max8/vignettes/gen~_operators
- Max 8 ships an example `gen~.pitchshift.maxpat`, described as "Overlap-add pitch shifting with stochastic blur", and `gen~.slicer.maxpat`, "Pitch-shift by overlap-add". **[extract]** https://docs.cycling74.com/legacy/max8/vignettes/gen_topic
- How it works **[inference, High; standard technique]**: two (or more) read taps sweep through a short delay line at the speed that gives the pitch ratio, each faded by a window, and each jumps back when it reaches the end of the window. Pitch drops; time does not stretch; nothing accumulates.
- **Latency:** the delay between input and output sweeps between near zero and the window length (typically 20 to 100 ms), averaging about half the window. This is smear, not a fixed processing latency, so it is not normally reported to Live. **[inference]**
- **CPU:** a few interpolated reads and a window per sample per channel. Cheapest real pitch-down option. **[inference]**
- **Sound:** the usual "harmoniser" grain and slight flutter, which is arguably on genre for slushwave. Larger windows sound smoother but smear more. **[inference, Low]**

### 1.3 `pitchshift~` and `retune~` (pitch down, keep time)

- Both arrived in Max 7 with a "zero-latency" time and pitch engine for `groove~` and `sfplay~`; `pitchshift~` is "a standalone object that can be used anywhere in the audio signal path"; `retune~` does "real-time auto-tuning, optimized for single-channel vocal and instrumental material". **[extract]** https://cycling74.com/articles/time-and-pitch-manipulation-in-max-7
- `pitchshift~` (Max 8): Ztx-based real-time pitch shifter. Attributes include `pitchshift` (a factor, so 0.75 = down about 5 semitones), `quality` (`basic`, `good`, `better`, `best`), `constantlatency` (no further description found) and `reportlatency`. `getlatency` reports `latency <samples>` out the right outlet. **[extract]** https://docs.cycling74.com/legacy/max8/refpages/pitchshift~
- `retune~` (Max 8): `quality` (`basic` default to `best`), `notelist` (Scala format scale), experimental `pitchdetection`, and the same `reportlatency`/`getlatency` pair. It is built to snap to a scale, not to apply a fixed transposition, so `pitchshift~` is the right one of the two for Slush Wave. **[extract]** https://docs.cycling74.com/legacy/max8/refpages/retune~
- **Latency:** no published figure. A Max for Live forum user measured "around 4000 samples" with `pitchshift~` (about 83 ms at 48 kHz), with replies saying to delay the dry path to match and report the latency to the host. Another reports "a few milliseconds" even at `retune~`'s basic quality. **[extract, forum, Medium]** https://cycling74.com/forums/m4l-device-with-pitchshift-is-causing-latency, https://cycling74.com/forums/lower-latency-pitch-correction-than-retune~
- **Practical rule:** query `getlatency` in the actual device at the chosen `quality`, then set Defined Latency to that value (section 5). **[inference, High]**
- **CPU:** a commercial time-pitch engine, and `best` quality costs more. Expect the most expensive item in the chain. **[inference]**

### 1.4 `pfft~` (spectral pitch shift)

- Window size must be a power of 2, default 512, minimum 32, maximum 1048576; overlap is a power of 2, default 2, with 4 recommended for most uses; hop size = window / overlap. **[extract]** https://docs.cycling74.com/reference/pfft~/
- Documented latency: "for a 1024-sample FFT window with an overlap factor of 4, the hop size is equal to 256, and the overall delay from input to output is 768 samples", so delay = window minus hop. **[extract]** https://docs.cycling74.com/reference/pfft~/ (forum posts dispute how overlap affects it, so measure: https://cycling74.com/forums/fft-overlap-and-latency)
- Worked numbers **[inference]**: 2048 window, overlap 4: 1536 samples = 32 ms at 48 kHz. 4096 window, overlap 4: 3072 samples = 64 ms.
- Pitch shifting in `pfft~` means writing a phase vocoder by hand (`cartopol~`, phase accumulation, bin remapping), or using the `gen~.spectraldelay` style in a `gen~` inside `pfft~`. More work than `pitchshift~` for a similar or worse result, and it smears transients. **[inference]**
- **CPU:** one FFT and one inverse FFT per hop per channel, plus per-bin maths. Moderate, rising with overlap. **[inference]**

### 1.5 `groove~` over a recording buffer (true slowdown)

- `record~` writes a signal into a `buffer~`; its `loop` attribute makes it continue from the start point when it reaches the end point, and a sync outlet ramps 0 to 1 through the recording. **[extract]** https://docs.cycling74.com/max8/refpages/record~
- `groove~` plays a `buffer~` (found by name, no patch cord) at a signal-rate speed: 1 is normal, 0 stops, negative plays backwards, and the speed may change continuously. Loop start and end are in milliseconds, `loopinterp` crossfades the loop seam, interpolation is cubic unless `timestretch` is on. **[extract]** https://docs.cycling74.com/max8/refpages/groove~
- With `timestretch 1`, `groove~` can change speed without pitch or pitch without speed (`pitchshift`, `pitchshiftcent`), with `mode` (including `efficient` and `extremestretch` for ratios above 2) and `quality`. The reference says "All modes are zero latency". **[extract]** https://docs.cycling74.com/max8/refpages/groove~
- Slush Wave wants slowing with the pitch drop (varispeed), which is plain `groove~` at speed `r` with `timestretch` off: no time-stretch engine, cubic interpolation, cheap. **[inference, High]**
- **Bounding the lag:** loop-record into a fixed buffer and read it slowed, then reset the read position on a cycle (a bar count from `plugsync~` or Live's transport) with a crossfade. Section 1.1's arithmetic sets the buffer length. A read head reading audio that `record~` is overwriting will click; keep the read head behind the write head and crossfade at resets. **[inference]**
- The same can be done in `gen~` with a `data` or `buffer` and `peek`/`poke`, which gives sample-exact control of the write and read heads and the reset crossfade in one place. **[inference, Medium]**
- `stretch~` is not a real-time option: it changes a `buffer~` destructively and offline. **[extract]** https://docs.cycling74.com/max8/refpages/stretch~
- **Latency:** none in the processing sense; the musical lag is the design. It should not be reported as Defined Latency (Live would shift the whole track by it). **[inference, High]**

### 1.6 MSP delay objects for comparison

- `tapin~`/`tapout~` cannot go below one signal vector of delay ("any delay less than 64 samples will be limited to a minimum of 64 samples" at vector 64), so short feedback loops belong inside `gen~`. **[extract]** https://docs.cycling74.com/legacy/max8/refpages/tapout~, https://docs.cycling74.com/legacy/max8/tutorials/15_delaychapter06
- This matters for wow and flutter, comb and allpass work, not for the pitch-down taps themselves. **[inference]**

## 2. Reverb

| Option | What it is | Where it lives | Notes |
|---|---|---|---|
| `gen~ gigaverb` | gen~ port of the gigaverb algorithm | Max 8 example `gen~.gigaverb.maxpat`; a stereo-input version is in the gen examples folder | Big, lush, cheap; a classic "hall" sound. Freeze or copy into the device. |
| `gen~ freeverb` | gen~ port of Freeverb | Max 8 example `gen~.freeverb.maxpat`, in the search path so `[gen~ freeverb]` loads it | "Might not be 1:1 exactly equal to freeverb~" (forum). |
| `yafr2` | Plate reverb abstraction after Dattorro (1997, Griesinger style) | Max examples, `examples/effects/reverb/lib/yafr2.maxpat` (path given for Max 6; Max 8 path unconfirmed) | Plain MSP abstraction, editable without a gen licence. |
| `yafr` | Older "Yet Another Free Reverb" example | Max examples | Forum users prefer `yafr2`. |
| `cverb~` | "Simple monaural reverberator"; argument and right inlet set reverb time in ms; `clear` message | Built-in MSP object | Mono and simple. Too plain to be the main slushwave reverb. |
| Hand-built in `gen~` | FDN or Dattorro plate with modulated delays | Your own code | Full control (long tails, modulation, damping); more work. |

Sources: gen~ example list **[extract]** https://docs.cycling74.com/legacy/max8/vignettes/gen_topic; freeverb and gigaverb externals no longer distributed, gen~ versions in the examples **[extract, forum]** https://cycling74.com/forums/freeverbgigaverb, https://cycling74.com/forums/problem-with-gigaverb~-i-need-a-stereo-in-reverb; opening `gen~.gigaverb` via Help > Examples > gen **[extract]** https://rnbo.cycling74.com/learn/gen-rnbo; yafr2 path and Dattorro basis **[extract, forum, Medium]** https://cycling74.com/forums/original-yafr-object, https://cycling74.com/forums/reverb-enquiry-cpu-light-ms-control-mspvst; list of reverbs that ship with Max (yafr, yafr2, gen~ freeverb, cverb~ and others) **[extract, forum, Medium]** https://cycling74.com/forums/reverb-in-max-msp; `cverb~` **[extract]** https://docs.cycling74.com/legacy/max8/refpages/cverb~

- There is no `yafr~` object; `yafr` and `yafr2` are abstractions (patches), not externals. **[extract, Medium]**
- **Latency:** all of these are feedback delay networks with zero processing latency. Pre-delay is part of the sound and is not reported. **[inference, High]**
- **CPU:** a few dozen delay reads and filters per sample. Low next to `pitchshift~`; longer tails do not cost more CPU, only more memory. **[inference]**
- **Recommendation [inference]:** start the spike with `gen~ gigaverb` (closest to the long, washed hall the genre wants; see [slushwave-sound.md](slushwave-sound.md)), copied into the device. Move to a hand-built `gen~` plate only if gigaverb's character or controls fall short. Use `yafr2` if `gen~` editing is not licensed.

## 3. Chorus, wow, flutter and phaser

- Max 8 ships `gen~.flange_chorus.maxpat` ("Flanger and Chorus"), and the MSP tutorials cover flanger and chorus with variable delays. **[extract]** https://docs.cycling74.com/legacy/max8/vignettes/gen_topic, https://docs.cycling74.com/max8/tutorials/15_delaychapter04
- All three are one `delay` in `gen~` read at a modulated time: chorus about 10 to 30 ms base with a slow LFO of a few ms depth; wow about 0.5 Hz; flutter about 6 Hz and up, shallower ([slushwave-sound.md](slushwave-sound.md) for settings). Use `cubic` or `spline` interpolation so the moving read does not dull or alias the top end. **[inference, High]**
- Random or drifting wow (smoothed noise into the delay time) sounds more like tape than a sine. **[inference, Low]**
- **Phaser** (essential for slushwave per the sound research, though not in this ticket's list): no dedicated Max 8 phaser object was found. Build a chain of first-order allpass filters in `gen~` with a swept coefficient and feedback; `allpass~` in MSP is a delay-based allpass, not the swept first-order kind. **[inference, Medium]**
- **Latency:** none to report. The base delay is part of the effect. Note pitch modulation from a moving delay is the point here, not a fault. **[inference, High]**
- **CPU:** very low; one or two interpolated reads per channel, plus an LFO. **[inference]**

## 4. Filtering and saturation

- `svf~`: state-variable filter with lowpass, highpass, bandpass and notch outputs at once. **[extract]** https://docs.cycling74.com/legacy/max8/refpages/svf~
- `biquad~`: two-pole two-zero filter; coefficients `a0 a1 a2 b1 b2` as floats or signals; `smooth` attribute (default 1) smooths coefficient changes. **[extract]** https://docs.cycling74.com/legacy/max8/refpages/biquad~
- `filtercoeff~`: computes `biquad~` coefficients at signal rate from frequency, gain and Q or slope (maths shared with `filtergraph~`), with five signal outlets into `biquad~`; sample-accurate, so modulated cutoffs stay clean. **[extract]** https://docs.cycling74.com/max8/refpages/filtercoeff~
- `lores~` (resonant lowpass) and a `gen~` version of it ship too; Max 8 also ships `gen~.filters.maxpat` ("Filter implementations"). **[extract]** https://docs.cycling74.com/legacy/max8/vignettes/gen_topic, https://rnbo.cycling74.com/learn/gen-rnbo
- `overdrive~`: soft clipping with a drive factor (argument or right inlet, also as a signal), default 1.0 = no distortion, suggested range about 1 to 10; values below 1 give very heavy distortion. **[extract]** https://docs.cycling74.com/max8/refpages/overdrive~
- In `gen~`, `tanh` or a polynomial soft clipper with a drive gain gives tape-like warmth with more control (for example asymmetry and a pre/de-emphasis filter around it). **[inference, Medium]**
- **Latency:** zero for all of these. Only oversampling the saturator (not built into `overdrive~`) would add latency. **[inference, High]**
- **CPU:** a handful of multiplies per sample. The cheapest stages in the chain. **[inference]**
- **Recommendation [inference]:** `svf~` or `filtercoeff~` into `biquad~` for the low-pass and high-pass, `overdrive~` for a first pass at saturation. Move both into the same `gen~` as the wobble later if the CPU or structure calls for it.

## 5. Reporting latency to Live

- Set **Defined Latency** in the Patcher Inspector, in samples; Live compensates for it on playback. Hovering over the device title bar shows the device latency in Live's status bar. **[read]** https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md (section "Latency")
- The Max docs: "A device can provide latency information to the Live host application" so Live can align tracks; set it only "when your patch analyzes a block of samples before producing output". If delay is the intended effect, "you don't need to set any latency". **[extract]** https://docs.cycling74.com/max8/vignettes/live_audiodevices, https://docs.cycling74.com/max5/vignettes/core/live_audiodevices.html
- In the patcher JSON this is the `"latency"` key (TAP's patcher has `"latency": 0`), so `tools/amxd.mjs` can set it from text. See [amxd-audio-effect.md](amxd-audio-effect.md). **[read]**
- **Changing it at runtime:** not documented. Forum reports: a `latency <samples>` message to `thispatcher` is the suggested route; a value sent at load did not stick until it was deferred; a `latency` message to `live.thisdevice` did not work for one user. **[extract, forum, Low]** https://cycling74.com/forums/accessing-defined-latency-of-device, https://cycling74.com/forums/trying-to-implement-basic-audio-lookahead-via-latency-reporting-am-lost
- **Reading it back:** the LOM `Device` has `latency_in_samples` and `latency_in_ms` (get, observe). The current reference is for Live 12; the extract suggests the Max 8 LOM page lists it too, but whether Live 11 has it is unconfirmed. **[extract, Medium]** https://docs.cycling74.com/apiref/lom/device/, https://docs.cycling74.com/legacy/max8/vignettes/live_object_model
- Ableton: Live's Delay Compensation offsets all tracks to keep them in sync; Max for Live devices "will also introduce additional latency when their editor window is open". **[extract]** https://help.ableton.com/hc/en-us/articles/209072409-Delay-Compensation-FAQ, https://help.ableton.com/hc/en-us/articles/360010545559-How-Latency-Works
- **What Slush Wave would report [inference]:** zero for a `gen~` delay-line pitch shifter, `groove~` slowdown, reverb, modulation, filters and `overdrive~`. A fixed figure only if it uses `pitchshift~` (its `getlatency` value) or `pfft~` (window minus hop). Any dry/wet mix must delay the dry path by the same amount, or the mix will comb. If the quality setting changes the latency, either fix the quality or update the latency through `thispatcher` and test it.

## 6. Measuring CPU

- Ableton's `maxdevtools` includes a **CPU Reporter** device: duplicate the device under test until Live's CPU meter is about 50%, record average and peak, compare before and after a change, and repeat at different buffer sizes. **[read]** https://github.com/Ableton/maxdevtools/blob/main/cpu-reporter/README.md
- Ableton's checklist: "The device does not cause unexpectedly high CPU load in the context of a Live Set." **[read]** https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md
- `gen~` warns that denormal maths can cost "up to 100 times" more; delay and history apply `fixdenorm` by default, which matters for long reverb tails decaying to silence. **[extract]** https://docs.cycling74.com/max8/vignettes/gen_overview

## 7. What ships with Max 8 versus what needs a package

| Item | Status |
|---|---|
| `gen~`, `delay`, `data`/`buffer`, `peek`/`poke` operators | Ships with Max 8 (Gen is part of Max). |
| gen~ examples (`gigaverb`, `freeverb`, `pitchshift`, `slicer`, `flange_chorus`, `filters`, `lores`, `spectraldelay`) | Ship as Max examples. Not inside a device until frozen or copied in. |
| `yafr`, `yafr2` | Ship as Max example abstractions. Same caveat. |
| `pitchshift~`, `retune~`, `stretch~`, `groove~` timestretch | Ship with MSP since Max 7. |
| `pfft~`, `record~`, `groove~`, `buffer~`, `tapin~`/`tapout~`, `cverb~`, `svf~`, `biquad~`, `filtercoeff~`, `lores~`, `overdrive~` | Core MSP. |
| Third-party (for example `elastic~`, Ircam or FluCoMa externals) | Packages; not needed for any stage. Avoid. |

- Freezing "makes sure that it contains all the files it needs to operate", including abstractions and third-party externals; use **List Externals and Subpatcher Files** to check. **[read]** https://github.com/Ableton/maxdevtools/blob/main/m4l-production-guidelines/m4l-production-guidelines.md
- Whether Live 11's bundled Max includes the full examples folder (so `gen~ gigaverb` resolves while developing) is **unconfirmed**. Safer: copy the gen code into the device's own `gen~` (embedded patcher or codebox) so the device has no outside dependency. **[inference]**
- **Licence:** Cycling '74: with only a Max for Live authorisation you can load and run devices that use Gen and MC, but "in order to save devices that use Gen, MC, and/or Jitter Geometry objects you must own a standalone Max license", and Gen code export is unavailable. Ableton: "It's only possible to edit gen patchers in Live provided you have a standalone or crossgrade Max license." **[extract]** https://support.cycling74.com/hc/en-us/articles/360049845974-Common-questions-about-Max-for-Live, https://help.ableton.com/hc/en-us/articles/360003262599-Editing-gen-patchers
- If there is no standalone licence, every stage still has a non-gen route: `groove~`/`record~` slowdown, `pitchshift~` pitch, `yafr2` or a hand-patched MSP reverb, `tapin~`/`tapout~` modulation (minimum one vector of delay), `svf~`/`biquad~` filters and `overdrive~`. **[inference, Medium]**

## Open questions for the spike

- Does the user have a standalone Max licence (decides `gen~`)? Does packing gen code from text with `tools/amxd.mjs` load and run in Live 11 without one?
- Actual `pitchshift~` latency at each `quality` (`getlatency`) on the target machine.
- Does `thispatcher` `latency` change Defined Latency at runtime in Live 11?
- Does Live 11's bundled Max include `examples/gen` and `examples/effects/reverb/lib`?
