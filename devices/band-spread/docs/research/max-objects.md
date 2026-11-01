# Max 8 objects for cutting the Band and shifting its Copies

Ticket: [#23 Which Max 8 objects can cut a narrow band and shift its copies?](https://github.com/jimmyNicholas/audio-plugins/issues/23), part of [Map: Band Spread](https://github.com/jimmyNicholas/audio-plugins/issues/22).

Researched 2026-11-01. Target: a Max for Live audio effect in Live 11 Suite with its bundled Max 8 (8.5.x), Intel Mac, macOS 13. Hard rules: stock objects only, no `gen~`, no `js`, no packages. Glossary terms (**Band**, **Copy**, **Shift**, **Spread**) are from [CONTEXT.md](../../CONTEXT.md).

## How to read this

- **Method caveat:** `docs.cycling74.com` and `cycling74.com` were blocked by the session's network proxy, as they were for the Slush Wave research. The Cycling '74 reference pages were read instead from their source files, the `*.maxref.xml` refpages that ship inside the Max application, as mirrored in the GitHub repository [markusepperer/Maxmsp_Knowledgebase](https://github.com/markusepperer/Maxmsp_Knowledgebase/tree/main/refpages_html/msp-ref) (`refpages_html/msp-ref/<object>.maxref.xml`). These are Cycling '74's own text, but the mirror does not say which Max version it was copied from (one page carries an `introduced 8.3.0` tag, so it is Max 8.3 or later, possibly Max 9). Marked **[refpage]**.
- **[extract]**: a search-engine extract of the named `docs.cycling74.com` Max 8 page (not read directly).
- **[measured]**: measured by the user in Live 11 in the Slush Pitch Layer audition ([RESULTS.md](../../../slush-wave/prototype/pitch-layer-audition/RESULTS.md)). The strongest evidence here, because it is the real target machine and Max version.
- **[forum]**: a Cycling '74 forum post, secondary.
- **[inference]**: the researcher's own reasoning from filter and signal theory. Not confirmed by any source.
- No source publishes CPU figures for any of these objects.

## Answer in short

| Need | Best stock object | Width or amount | Latency | Notes |
|---|---|---|---|---|
| Cut the **Band** | `filtercoeff~ bandpass` into two or three `biquad~` in series | Q (centre / bandwidth) | Zero | Signal-rate coefficients, so centre and width can move without zipper noise. Steepness grows with each stage. |
| **Shift** by a fixed number of Hz | `freqshift~` | Hz, signal or float | Zero processing latency [inference] | Two outlets: shifted up and shifted down. Shifting below 0 Hz folds back up. |
| **Shift** by a pitch ratio | `pitchshift~` | Factor (or cents) | 4096 samples at every Quality [measured] | All instances share one latency, so the copies line up with each other. |
| Spectral alternative | `pfft~` with `gizmo~` (pitch) or `fbinshift~` (Hz) | Ratio or Hz | Window minus hop (eg 3072 for 4096 / 4) | Can also cut a brick-wall Band by zeroing bins. More work, smears transients. |

## 1. The Band: which filter

All four candidates are two-pole (second-order) band passes, so on their own each falls at about 6 dB per octave either side of the centre, and a narrow one rings. Only stacking stages makes the skirts steeper. **[inference]**

### 1.1 `svf~` (state-variable filter)

- Chamberlin state-variable filter with four simultaneous outlets: low pass, high pass, **band pass (third outlet)**, notch. Inlets: signal; cutoff frequency (Hz, signal or float); resonance (signal or float, labelled "Resonance (0-1)"). Arguments: centre frequency (default 0), resonance (default 0.01), and an input mode `Hz` (default), `linear` or `radians`. Messages: `Hz`, `linear`, `radians`, `clear`. **[refpage]**
- **Range limit:** "By default, the center frequency is expressed in Hz, where the allowable range is from 0 to one quarter of the current sampling rate." At 48 kHz that is 12 kHz, so a Band above 12 kHz is impossible. **[refpage]**
- **Width:** the refpage is inconsistent. The inlet is labelled "Resonance (0-1)", and the right-inlet text says Q "is defined as the filter bandwidth divided by the center frequency. Useful Q values for svf~ are typically between 0.01 and 1." So it is not a true Q, and the mapping from this number to a width in Hz is undocumented. **[refpage]**
- **Clicks:** floats "are sampled once every signal vector", so stepping the centre with floats gives zipper noise; drive it with a signal (`line~`) for clean moves. **[refpage]**, **[inference]**
- **Verdict:** usable for a rough Band, but the sr/4 ceiling and the vague width scale rule it out as the main filter. **[inference]**

### 1.2 `reson~` (resonant band pass)

- Inlets: signal; input gain; centre frequency (Hz); Q. All signal or float. Arguments: initial gain (default **0**, so it is silent until a gain is given), centre (default 0), Q (default 0.01). Also attributes `gain`, `cf`, `q`, a `list` (gain, centre, Q) and `clear`. **[refpage]**
- **Width:** a true Q: "Q = cf/bandwidth". "Useful Q values are typically between 0.01 and 500." **[refpage]**
- **Equation:** `y[n] = a0 * (x[n] - r * x[n-2]) + b1 * y[n-1] + b2 * y[n-2]`. Gain "should generally be less than 1". **[refpage]**
- **Level:** the refpage does not say whether the peak gain stays at unity as Q changes. A classic resonator of this form gets louder as Q rises, so a narrow Band would need gain compensation. **[inference]**
- **Clicks:** centre and Q accept signals, so a `line~` gives smooth moves. **[refpage]**, **[inference]**

### 1.3 `filtercoeff~` into `biquad~`

- `biquad~`: `y[n] = a0*x[n] + a1*x[n-1] + a2*x[n-2] - b1*y[n-1] - b2*y[n-2]`. Six inlets: signal, then `a0 a1 a2 b1 b2`, each signal or float. Arguments are the five coefficients. Messages: `list` (five coefficients), `clear`, `dictionary <name>` (a `filterdesign` dictionary), `stoke` (preload state). Attribute `smooth` (default 1, "Smooth Coefficient Changes"). **[refpage]**
- `filtercoeff~`: "a signal-rate filter coefficient calculator for the biquad~ object" from frequency, gain and Q (or slope S for shelves). Three inlets (frequency, gain, Q; signal or float), **five signal outlets** that go straight into `biquad~` inlets 2 to 6. Arguments: default filter type (symbol, eg `bandpass`) and an optional resampling factor. Type messages include `bandpass`, `resonant`, `gainbpass`, `gainresonant`, `lowpass`, `highpass`, `bandstop`, `peaknotch`, `allpass`, `off`. **[refpage]**
- **Band pass vs resonant:** `bandpass` and `resonant` both take centre and Q, and both set "the gain parameter ... to unity gain (1.0)". The refpage does not draw the curves. By analogy with the RBJ cookbook (which `filtergraph~`'s maths is widely said to follow), `bandpass` should be the constant 0 dB peak band pass, which keeps the Band's level steady as the width changes. Confirm by ear or with a spectrum analyser. **[refpage]** for the messages, **[inference]** for the curve.
- **Width:** a true Q (centre / bandwidth); a width in Hz converts as `Q = centre / width`, and a width in octaves maps to a constant Q. **[inference]**
- **Clicks:** coefficients are signals, recomputed every sample, so a smoothed centre or Q (via `line~`) moves cleanly. `biquad~`'s `smooth` attribute smooths float coefficient changes too. **[refpage]**, **[inference]**
- **Steeper:** one `filtercoeff~` can feed several `biquad~` objects in series with the same five coefficient signals. Each stage adds about 6 dB per octave per side (two stages about 12, three about 18), and the band narrows a little with each stage, so the Q should be lowered a little to keep the same width. **[inference]**

### 1.4 `cascade~`

- A series of biquads in one object. Inlets: signal, and a list of coefficients "in sets of five, each set corresponding to a second-order section". Messages: `list`, `bypass <0/1>`, `zero <0/1>` (mute), `clear`, `dictionary <name>`. **[refpage]**
- **Catch:** coefficients come only as a list (no signal inlets), and `filtercoeff~` outputs signals, so `cascade~` cannot be fed by `filtercoeff~` directly. The usual partner is `filtergraph~` or `filterdesign`, which output lists. Each centre change is a jump in coefficients, which on a narrow Band can click; there is no documented smoothing on `cascade~`. **[refpage]** for the inlets, **[inference]** for the click risk.
- **Verdict:** stacked `biquad~` objects fed by one `filtercoeff~` do the same job with signal-rate control. **[inference]**

### 1.5 Band filter comparison

| Object | Width as | Steepness | Range | Centre change |
|---|---|---|---|---|
| `svf~` (band pass outlet) | "Resonance" 0 to 1, scale undocumented | 2-pole | 0 to sr/4 only | Floats per vector (zipper); smooth with a signal |
| `reson~` | Q = centre / bandwidth, 0.01 to 500 | 2-pole | Full | Smooth with a signal; level may rise with Q |
| `filtercoeff~` + `biquad~` | Q | 2-pole per stage; stack for steeper | Full | Signal-rate coefficients, smooth |
| `cascade~` | Coefficient lists | Any number of stages | Full | List jumps, may click |

## 2. Frequency shift: `freqshift~`

- "A time-domain frequency shifter (also known as a single-sideband ring modulator)." **[refpage]**, also **[extract]** of the Max 8 page `docs.cycling74.com/legacy/max8/refpages/freqshift~`.
- **Inlets:** left, the signal to shift; right, "Amount of Frequency Shift (Hz)", signal or float (int converted to float). **[refpage]**
- **Outlets:** left, "Frequency-shifted Signal (Positive Sideband)"; right, "Frequency-shifted Signal (Negative Sideband)". **[refpage]** So one `freqshift~` set to +200 Hz gives the input moved up 200 Hz at the left and down 200 Hz at the right; two **Copies** from one object. **[inference]**
- **Argument:** the shift in Hz, default 0. **Messages:** `clear` ("in case the filter is blown up"), `resetphase` (resets the phase of the two internal oscillators). **[refpage]**
- **Units:** Hz, positive or negative (a negative amount swaps which outlet goes up). **[refpage]**, **[inference]**
- **Latency:** not stated. It is built on a quadrature (Hilbert) filter pair, the same idea as `hilbert~` ("a 6th order hilbert transformer"), which is recursive and adds phase shift but no block of latency. Treat it as zero for alignment. **[inference]** (`hilbert~` text is **[refpage]**)
- **Below 0 Hz:** a component pushed below 0 Hz reappears as a positive frequency (folds back up through zero): a 50 Hz sine shifted by -100 Hz comes out at 50 Hz. Reported on the forum for a Gen version, and it follows from the maths. **[forum]** https://cycling74.com/forums/hilbert-or-freqshift-in-gen, **[inference]**
- **Above Nyquist:** a component pushed above half the sample rate aliases back down (reflects off Nyquist). Not documented. **[inference]**
- **Character:** every partial moves by the same number of Hz, so harmonic ratios are broken (bell-like, inharmonic), except a single sine, which simply moves. For a narrow Band that is close to a sine, the difference from a pitch shift is smaller than on full-range audio. **[inference]**
- **CPU:** a few filter stages and two oscillators per sample; far cheaper than `pitchshift~`. **[inference]**

## 3. Pitch shift: `pitchshift~`

- "Ztx-based real-time pitchshifting." Inlets: left, the signal; right, a pitch factor as a signal (or cents when `usecents` is on). Outlets: left, the shifted signal; right, "Current latency, reported in samples". Argument: number of channels (default 1). **[refpage]**
- **Attributes:** `pitchshift` (factor, default 1.0; 2.0 is an octave up, 0.5 an octave down), `quality` (`basic` "the default", `good`, `better`, `best`), `reportlatency` (on or off), `constantlatency` (on or off, undocumented beyond its label), `enabled`, `usecents`. The mirror also lists `pitchshiftcent` (cents); the search extract of the Max 8 pages shows `pitchshiftcent` on `mc.pitchshift~` but not on `pitchshift~`, so in Max 8 send a factor to `pitchshift`, not cents. **[refpage]**, **[extract]** https://docs.cycling74.com/legacy/max8/refpages/pitchshift~
- **Messages:** `getlatency` makes it send `latency <samples>` from the right outlet. (The refpage text oddly says "getlatency, followed by a zero or one, toggles reporting"; the audition sent a bare `getlatency` with `@reportlatency 1` and it worked.) **[refpage]**, **[measured]**
- **Confirmed in Max 8 by the audition:** `pitchshift~ @quality good @reportlatency 1` loads, `pitchshift <factor>` pitches correctly (octave down read -5 cents, fifth down -2 cents on a tuner), `quality <name>` gave no Max Console error, and `getlatency` reported **4096 samples (92.9 ms at 44.1 kHz)**. **[measured]**
- **Does Quality change latency?** The readout stayed at 4096 at every Quality and no difference in sound was heard. But the user also noted the readout "isn't updating" on a Quality change, so it is not proven whether latency is truly fixed or the re-measure did not fire. The best current reading is that latency is 4096 at every Quality. **[measured]**, unresolved.
- **Copies line up:** every `pitchshift~` at the same settings reports the same latency, so six **Copies** made with it stay in time with each other, and Band Spread has no dry path to align against. Whether to report the 4096 to Live as Defined Latency is a separate open question on the map. **[inference]**
- **CPU:** the audition's Combined device (at most four mono `pitchshift~` running, plus delay-line engines) read 6 to 7% on Live's meter at a 512-sample buffer, and 20 copies of the track used about 15%. Six mono instances for Band Spread should be in the same range, well within budget. **[measured]** for the readings, **[inference]** for six.
- Six instances: use six mono `pitchshift~` (one per **Copy**) on the mono **Band**, then pan after. `mc.pitchshift~` exists in Max 8 but was not tried. **[extract]**, **[inference]**
- **Edges:** `pitchshift~` scales frequencies, so a Band shifted down approaches but never crosses 0 Hz. Shifted up past Nyquist, the documentation does not say what happens; expect the top to be lost or aliased. **[inference]**

## 4. Other stock ways to shift a Band

### 4.1 `pfft~` spectral processing

- `pfft~ <subpatch> <FFT size> <overlap>`; FFT size a power of 2 (default 512), overlap a power of 2 (default 2, "4 is recommended for most applications"). I/O delay "is equal to the window size minus the hop size (e.g., for a 1024-sample FFT window with an overlap factor of 4 ... the overall delay from input to output is 768 samples)". `mute <0/1>` stops processing. Inside, `fftin~` and `fftout~` give the spectral frames. **[refpage]**
- **`gizmo~`**, "frequency-domain pitch shifter for pfft~": finds spectral peaks and moves them. Inlets: real, imaginary (from `fftin~`), pitch scalar (float, 1.0 = none). Messages `clip <0/1>` (clip or fold out-of-range peaks) and `freqshift <Hz>` (adds a frequency offset after scaling). Must be used "inside a pfft~ with an overlap of 4 or more". **[refpage]**
- **`fbinshift~`**, "frequency domain frequency shifter for pfft~": shifts all bins by a number of Hz (right inlet, float), positive up, negative down. **[refpage]**
- **Band cut for free:** inside a `pfft~`, the Band can be cut brick-wall by zeroing every bin outside the centre and width, far steeper than any biquad stack, then shifted copies made from the same frame. **[inference]**
- **Trade-offs [inference]:**
  - Latency is window minus hop, eg 3072 samples for 4096 / 4, similar to `pitchshift~`.
  - Frequency resolution is sample rate / window, eg about 11.7 Hz per bin at 48 kHz with 4096; a narrow low Band covers only a few bins.
  - Transients smear, and the patch is much more work to build and debug without being able to run Max in the cloud sessions.
  - Hard to make six independent Copies in one `pfft~` with stock objects alone (six `gizmo~` in parallel inside one subpatch may work, untested).
- **`fft~`** with `ifft~` is the older, manual route: no windowing or overlap is managed for you and its delay is more than twice that of `pfft~` ("less than half the delay than with an fft~ / ifft~ combination"). Not worth it here. **[refpage]**

### 4.2 Not suitable

- `retune~` snaps to a scale (pitch correction), not a fixed transposition. **[refpage]**
- Delay-line pitch shifters are normally built in `gen~` (ruled out); the MSP version from the audition did not load in round 1 (it used `expr~`, which is not a Max object) and was fixed with `expr` at control rate for round 2. **[measured]**

## 5. Still unconfirmed

- Whether `pitchshift~`'s latency changes with `quality` or `constantlatency` (the readout did not re-measure).
- The exact width scale of `svf~`'s resonance, and the curve shapes of `filtercoeff~`'s `bandpass` and `resonant` modes.
- `freqshift~` behaviour above Nyquist, and its exact latency (believed zero).
- Whether the mirrored refpages match Max 8.5 exactly (names that the audition already used are confirmed; others, like `usecents` and `pitchshiftcent`, are not).

## Recommendation for Band Spread

- **Band filter:** `filtercoeff~ bandpass` driving two `biquad~` in series (a third if the skirts are still too wide by ear). Centre and Q from the panel go through `line~` (about 20 ms) into `filtercoeff~`'s signal inlets. Show width on the panel in octaves or Hz and convert to Q in the patch (`Q = centre / width`). This keeps the Band's level steady, covers the full range (unlike `svf~`), and moves without clicks (unlike `cascade~`).
- **One Band, six Copies:** sum the input to mono, cut the Band once, then fan it out to six Copy subpatchers, each with its own `mute~` (so a Copy that is off costs nothing), Shift, level, pan and mono, as in the audition.
- **Frequency Shift engine:** one `freqshift~` per Copy, Shift in Hz into its right inlet (smoothed with `line~`), taking the left (positive sideband) outlet and letting a negative Shift move the Copy down. Zero latency. A Copy shifted below 0 Hz will fold back up, and one past Nyquist will alias, so the Shift range ticket needs to clamp it.
- **Pitch Shift engine:** one mono `pitchshift~ @quality good @reportlatency 1` per Copy, Shift as a factor `2^(semitones / 12)` sent as `pitchshift <factor>` (no cents attribute, which is unconfirmed in Max 8). All six share the same 4096-sample latency, so the Copies stay aligned; measure it with `getlatency` and show it on the panel as the audition did.
- **For the A/B prototype:** build both engines inside each Copy behind a shared Engine switch, each in its own subpatcher with a `mute~`, so only one engine runs at a time. Because there is no dry path, the only audible difference in timing is that the pitch engine starts 4096 samples (about 93 ms) later; for a fair A/B, delay the frequency engine by 4096 samples with `delay~` (or report the latency for both) so switching does not jump in time.
- Keep `pfft~` (`gizmo~` or `fbinshift~`, with a brick-wall Band) as a later experiment if the biquad Band is not narrow enough by ear.
