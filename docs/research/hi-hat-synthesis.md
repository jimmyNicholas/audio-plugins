# Research: how hi-hats are synthesised, and which approach suits a breeder

Researched 2026-11-12 for [#32](https://github.com/jimmyNicholas/audio-plugins/issues/32), part of the map [#29 "Map: Bloodline Breaks"](https://github.com/jimmyNicholas/audio-plugins/issues/29): a sample-free hip hop drum album, with one JUCE plugin holding Kick, Snare and Hats, each found by breeding (see the [Kick Breeder spec](../../devices/kick-breeder/docs/SPEC.md), the [Snare Breeder spec](../../devices/snare-breeder/docs/SPEC.md) and [shared/breeder](../../shared/breeder/README.md)).

The question: how are closed and open hi-hats synthesised without samples, and which approach gives useful variety from a small set of settings stored as 0 to 1?

## How to read this

- **Method:** web search extracts only. Full page reads were blocked in this session (the network does not reach soundonsound.com, ccrma.stanford.edu or the ICMC proceedings host), so the primary papers were read only as quoted in search results. Every claim names its source.
- **Tags:** **[source]** is backed by the linked page as quoted in search. **[forum]** comes from a forum or hobbyist page only. **[inference]** is the researcher's reasoning, not checked against a source. **[unverified]** is from memory or a single weak source and should be checked before it is built on.
- **Targets:** "C++" means a JUCE plugin (the album plugin). "MSP" means plain Max 8 objects without `gen~`, as the current spikes are built.

## TL;DR

1. **The 808 recipe is the best base for a breeder.** Six detuned square waves, a band-pass, a high-pass and a short decay. It is cheap, it sounds like a hi-hat at almost any setting, and one Pitch setting that scales all six together moves it smoothly, which is what breeding needs.
2. **Mix in filtered noise.** Noise alone gives soft, breathy hats; squares alone give the 808's glassy metal. A Metal to noise balance spans both, and boom bap hats (mostly sampled from records, so noisy and dull) sit towards the noise end.
3. **FM, ring-mod and modal models are richer but harder to breed.** A small nudge to an FM ratio or a mode law can flip the whole timbre, so neighbours in the search do not sound like neighbours. Keep them for later.
4. **Choke comes free:** closed and open hats share one voice, as on the 808. A closed note restarts the shared envelope with a short decay, which cuts the open tail.
5. **Lo-fi is a settings job, not a model:** sample-rate and bit reduction (the SP-1200's 12-bit, 26.04 kHz with no reconstruction filter) plus a low-pass Tone. One Grit setting covers it.
6. **Recommended voice:** 14 settings (table below), with closed and open decays as separate settings and everything else shared.

## 1. The 808 hat circuit

- **Six square oscillators, shared.** The TR-808 cymbal and hi-hats start from six square-wave oscillators tuned enharmonically and mixed into a complex spectrum, built from one HD14584 hex Schmitt trigger inverter **[source: [SOS Synth Secrets, via search](https://www.soundonsound.com/techniques/synthesizing-realistic-cymbals); [Werner, Abel and Smith, ICMC 2014](https://speech.di.uoa.gr/ICMC-SMC-2014/images/VOL_2/1453.pdf)]**. The closed hat "shares the same sound source with the OH" **[source: [Roland TR-808 service notes, manualslib excerpt](https://www.manualslib.com/manual/1294765/Roland-Tr-808.html?page=6)]**.
- **Frequencies.** Werner, Abel and Smith list four fixed oscillators at 205.3, 304.4, 369.6 and 522.7 Hz, and two with trimpots whose ranges are 359.4 to 1149.9 Hz and 254.3 to 627.2 Hz, factory set to 800 and 540 Hz **[source: [Werner et al., quoted on KVR](https://www.kvraudio.com/forum/viewtopic.php?p=7494787); [Elektronauts](https://www.elektronauts.com/t/hh-lab-tips/207960)]**. One forum post calls 540 and 800 Hz the cowbell's frequencies **[forum: [e-licktronic](https://e-licktronic.com/forum/viewtopic.php?p=958)]**. Both can be true if the cowbell takes two of the same six oscillators **[inference]**. No two of the six are in a simple ratio, which is why the sum is metallic rather than a chord **[inference]**.
- **Filters.** The mix goes through band-pass filtering. The cymbal splits it into bands, each with its own high-pass stage **[source: Werner et al., via search]**. The hats go through one main band-pass, then a high-pass **[source: [Tiptop HATS808, a clone of the circuit](https://www.kraftmusic.com/tiptop-audio-hats808-analog-hi-hats-drum-module.html); [Misfit Audio Transistor 808 Hats manual](https://docs.cherryaudio.com/misfit-audio/transistor-808-hats), whose Body control sets the master high-pass]**. The centre frequencies were not found in any source reached this session; the band sits roughly in the 6 to 10 kHz region by ear **[unverified]**.
- **Envelopes and controls.** The 808 panel has Level and Decay for the open hat, and only Level for the closed hat **[source: [Roland TR-808 technical specifications](https://support.roland.com/hc/en-us/articles/201963539-TR-808-Technical-Specifications)]**. No millisecond figures were found. Closed hats are typically tens of milliseconds and open hats several hundred **[unverified]**.
- **Choke.** When the closed hat is triggered while the open hat sounds, "Q23 turns on ... the decay time of the OH circuit terminates" **[source: Roland service notes, OCR excerpt]**. A clone describes it as the closed hat's shorter envelope being forced on to the open hat's VCA **[source: Tiptop HATS808 listing]**.

**Settings it needs:** Pitch (one multiplier on all six), band-pass centre and width, high-pass cutoff, closed decay, open decay. **Useful ranges [inference]:** Pitch 0.5 to 2 times the 808 set (below 0.5 it turns gong-like, above 2 the oscillators' harmonics alias in naive C++), band-pass 3 to 12 kHz, high-pass 2 to 10 kHz. **CPU:** six oscillators and two filters per sample, very low in either target **[inference]**. **C++:** easy; use band-limited squares (PolyBLEP) or 2x oversampling, as naive squares at 800 Hz times 2 alias badly **[inference]**. **MSP:** easy; `rect~` is MSP's band-limited pulse oscillator **[source: [Max 8 rect~ reference](https://docs.cycling74.com/max8/refpages/rect~)]**, so six `rect~`, a `+~` mix, two `svf~` (band and high outputs) and a `line~` envelope into `*~`.

## 2. The 909 hats

The TR-909's hats and cymbals are not synthesised: they are 6-bit samples of a real kit (a Paiste top and a Zildjian bottom) read from ROM, then through an analogue VCA and low-pass **[source: [Wikipedia, TR-909](https://en.wikipedia.org/wiki/Roland_TR-909); [Attack Magazine via search](https://attackmagazine.com/?p=133892); [SOS via search](https://www.soundonsound.com/node/4918183)]**. One gear site gives 18 kHz playback from a 32 Kb ROM **[unverified]**. Noise Engineering also notes the 909 uses samples for its hats **[source: [Noise Engineering](https://noiseengineering.us/blogs/loquelic-literitas-the-blog/hats-off-to-this-series-synthesizing-hats-in-the-final-percussion-synthesis-post/)]**.

So the brief's "909 circuit" is out of reach for a sample-free album. What carries over is its **analogue shaping**: a decay envelope and a low-pass over a coarse, gritty source. That is what the Grit and Tone settings below imitate **[inference]**.

## 3. Filtered noise

- **How:** white noise, a band-pass or high-pass, a fast exponential decay. This is the basic hat in most synth tutorials **[source: [MusicRadar, noise-based hats in Live](https://www.musicradar.com/tuition/tech/make-noise-based-hi-hats-in-ableton-572822); [MusicTech, TAL-NoiseMaker](https://musictech.net/tutorials/weekend-workshop-subtractive-drum-synthesis-tal-noisemaker/)]**. A resonant high-pass thins the noise for open hats **[source: MusicTech]**.
- **Limit:** a cymbal's modes sit at discrete frequencies, so a real hat is not noise but a dense fog of inharmonic partials **[source: [SOS, Analysing Metallic Percussion, via search](https://www.soundonsound.com/techniques/analysing-metallic-percussion)]**. Noise alone sounds breathy and shaker-like rather than metal **[inference]**.
- **Settings:** filter centre, width or resonance, decay. **Ranges:** centre 3 to 12 kHz, decay 10 to 1500 ms **[inference]**. **CPU:** the cheapest option. **C++:** trivial. **MSP:** `noise~` into `svf~`; also already used for the snare's wires.

## 4. FM and ring-mod metal

- **FM:** a non-integer carrier to modulator ratio gives inharmonic partials; index sets brightness, and a falling index gives a bright-to-pure strike **[source: [Max 8 Synthesis Tutorial 5: FM](https://docs.cycling74.com/api/latest/max8/tutorials/06_synthesischapter05)]**. Noise Engineering builds hats from two oscillators in FM or PM, then a high-pass to strip the fundamental, a VCA and a decay **[source: [Noise Engineering, Creating FM hats](https://noiseengineering.us/blogs/loquelic-literitas-the-blog/creating-fm-hats/)]**. The SOS cymbal patch uses six oscillators as three FM pairs **[source: SOS Synth Secrets, via search]**, and shorter envelopes from the same patch give acceptable hi-hats **[source: same]**.
- **Ring-mod:** multiplying two non-harmonically related square waves gives sum and difference sidebands for every harmonic, so dense, metallic spectra **[source: [Wikipedia, Ring modulation](https://wikipedia.com/wiki/Ring_modulator)]**. The Corsynth DR-03 hats module builds hats from ring-modulated squares with band-pass and low-pass filters **[source: [Corsynth DR-03 manual](https://manuals.online/corsynth/synthesizer/dr-03-hi-hats-metal/user-manual/1g4yg)]**.
- **Settings:** carrier frequency, ratio (or second frequency), index, index decay, filter, decay. **Ranges [inference]:** carrier 300 Hz to 3 kHz, ratio 1.1 to 4 avoiding integers, index 1 to 20.
- **Breeding problem [inference]:** the timbre jumps as the ratio crosses near-integer values, so a 0.05 nudge can change a hat from clangy to bell-like. Variations then stop sounding like their parent, and the search stops converging. A breeder needs settings whose small changes make small sounds; FM's ratio is the opposite.
- **CPU:** low (two to six sine oscillators). **C++:** easy. **MSP:** easy: `cycle~`, `*~`, `+~` per pair; ring-mod is `rect~` times `rect~`.

## 5. Modal and physical models

- **Modal:** a bank of resonators, each a mode, struck by a noise burst. In MSP that is many `reson~` (or `biquad~`) on enveloped noise **[source: [Dobrian's Max Cookbook](https://music.arts.uci.edu/dobrian/maxcookbook/rhythmic-filtered-noise-bursts); [Cycling '74 forum, mc.reson~ cymbal](https://cycling74.com/ja/forums/problem-with-changing-mcreson-frequency-steep-q-or-cymbal-sounds)]**. Cook's PhISEM renders many random collisions as filtered noise bursts through resonances that can vary on each strike, used for tambourines and sleigh bells, but no Cook cymbal model was found **[source: [Cook PhISEM notes, McGill](https://caml.music.mcgill.ca/~gary/618/week12/phism.html)]**.
- **Physical:** cymbals need a nonlinear (von Kármán) plate model for crash build-up; the linear Kirchhoff model is cheaper but misses it **[source: [Bilbao, nonlinear plates](https://www2.ph.ed.ac.uk/~sbilbao/platepage.html); [Ducceschi, Touzé and Bilbao](https://perso.ensta-paris.fr/~touze/PDF/ducceschi_smac2013.pdf)]**. In a modal version, the number of modes kept is the crucial cost choice **[source: Ducceschi et al.]**.
- **Settings:** a mode law (base frequency, spread, stiffness), damping, strike brightness, and for real physics, size and thickness. **CPU:** a hat needs tens to hundreds of modes to sound dense **[source: SOS: "hundreds of energised modes"]**, so tens to hundreds of filters per voice; a finite-difference plate is far heavier and not for a drum plugin with many voices **[inference]**.
- **C++:** modal is straightforward (a loop of two-pole resonators), physical plate is a research project. **MSP:** modal is possible because `build-patch.mjs` generates the patch, so 40 `reson~` boxes cost no hand work, but CPU and patch size grow fast **[inference]**.
- **Breeding:** good variety, but most of it is cymbal and bell, not tight hip hop hats; worth trying later as a "Metal" upgrade **[inference]**.

## 6. Choke between closed and open

- **On the 808:** a closed trigger ends the open decay (section 1).
- **Simplest build [inference]:** one shared voice and one amplitude envelope. A closed note restarts the envelope with the closed decay; an open note with the open decay. Because there is only one envelope, a closed note always cuts the open tail. In MSP that is one `line~` fed a new breakpoint list, starting from its current value with a 1 to 2 ms ramp to avoid a click. In C++ it is one envelope object per hat voice, never polyphonic.
- **Open question:** a "pedal" or foot-chick note (a very short, dull closed hat) is common on real kits. It could be a third note using closed decay times 0.5 and Tone lowered, with no new settings.

## 7. Boom bap and lo-fi character

- **Hats were sampled, not synthesised.** Classic boom bap hats were chopped from records and played from the SP-1200 and MPC; the limitations (12-bit, low sample rates) became the aesthetic **[source: [Native Instruments, Hip hop drums 101](https://www.native-instruments.com/blogs/blog/hip-hop-drums-101); [Attack Magazine, 90s boom bap](https://www.attackmagazine.com/technique/beat-dissected/90s-boom-bap-hip-hop/)]**. Lifted from a record, a hat carries the record's noise and roll-off, so it is duller and noisier than an 808 hat **[inference]**.
- **The SP-1200** runs at 26.04 kHz, 12-bit, with drop-sample pitch shifting and no reconstruction filter, so imaging adds brightness and grit **[source: [Wikipedia, SP-1200](https://en.wikipedia.org/wiki/E-mu_SP-1200); [Rossum, SP-1200](https://www.rossum-electro.com/pages/education-sp-1200); [MusicRadar](https://www.musicradar.com/news/reissue-of-the-classic-sp-1200-sampler-announced-by-dave-rossum)]**. Whether the rate changed from 27.5 kHz in the SP-12 is contested **[source: Wikipedia talk page, via search]**.
- **In MSP:** `degrade~` reduces sample rate (as a ratio of the system rate) and bit depth **[source: [Max degrade~ reference](https://docs.cycling74.com/max8/refpages/degrade~)]**. A user reports little audible change above about 4 bits of reduction on hot signals **[forum: [Cycling '74](https://cycling74.com/forums/degrade-bit-range)]**. **In C++:** a sample-and-hold plus quantiser, a few lines.
- **Feel lives in the pattern, not the voice:** swing around 52 to 62 percent and velocity accents (on-beats louder than off-beats) **[source: [Audeobox, MPC drum programming](https://www.audeobox.com/learn/mpc-software/mpc-drum-programming/); [MusicRadar](https://www.musicradar.com/how-to/how-to-program-an-old-school-hip-hop-beat-in-your-daw)]**. So velocity should change the hat: level, plus a little brightness and length, as the kick spec proposes for click and drive **[inference]**.

## 8. Comparison

| Approach | Settings | Variety for a breeder | CPU | C++ (JUCE) | Plain MSP |
| --- | --- | --- | --- | --- | --- |
| 808 squares + band-pass + high-pass | 5 to 6 | Narrow but always a hat; smooth under nudges | Very low | Easy (band-limit the squares) | Easy: six `rect~`, two `svf~` |
| Filtered noise | 3 | Narrow, soft; smooth | Lowest | Trivial | Trivial: `noise~`, `svf~` |
| FM pairs / ring-mod | 4 to 6 | Wide, but jumps at ratio changes | Low | Easy | Easy: `cycle~`, `*~` |
| Modal bank | 4 to 6 (mode law) | Wide, leans cymbal and bell | Medium to high | Moderate | Possible via generated patch; heavy |
| Physical plate | Many | Wide; slow to explore | Very high | Hard | Not practical |
| Hybrid: 808 squares + noise + lo-fi | 14 | Wide and smooth: 808 glass to dusty record hat | Low | Easy | Easy |

## 9. Recommendation: the hat voice

A hybrid: the 808's six squares (with Pitch and Detune) mixed with noise, through a band-pass and a high-pass, one shared amplitude envelope with separate closed and open decays (so the choke is built in), a stick tick, then Grit (lo-fi), and the Comp, Sat and Tone stages the kick and snare already have. Built from plain MSP objects (no `gen~`), and a direct port to C++ **[inference throughout, based on sections 1 to 8]**.

| Part | Settings | Range (proposed) |
| --- | --- | --- |
| Metal | Pitch: one multiplier on the 808 set (205.3, 304.4, 369.6, 522.7, 540, 800 Hz); Detune: pulls each oscillator off its 808 ratio by a fixed, seeded offset | 0.5 to 2 times, exp; 0 to 1 (0 is the 808 set) |
| Balance | Metal to noise, equal power | 0 to 1 |
| Colour | Band-pass centre; band-pass width | 3 to 12 kHz, exp; wide to narrow (svf~ resonance 0 to 0.8) |
| Thin | High-pass cutoff | 1.5 to 10 kHz, exp |
| Decay | Closed decay; Open decay. Each note uses one, on one shared envelope, so closed chokes open | 10 to 150 ms, exp; 120 to 1500 ms, exp |
| Stick | A short tick of high-passed noise at the start | 0 to 1 |
| Grit | Sample-rate and bit reduction together: 0 is clean, 1 is about 11 kHz and 8 bits | 0 to 1 |
| Comp, Sat, Tone | As on the kick and snare | 0 to 1, 0 to 1, 2 to 16 kHz |

Fourteen settings, between the kick's thirteen and the snare's sixteen. Notes:

- **Why this breeds well:** every setting moves the sound smoothly, and almost every point in the space is still a hi-hat. Pitch scales the six squares together, so the 808's inharmonic ratios are kept unless Detune moves them **[inference]**.
- **Random ranges:** as the snare gives half of all random snares one burst, give about half of all random hats Detune near 0 (an 808-family hat) and Grit near 0, so the first round is not all dusty or all alien **[inference]**.
- **Velocity:** level, plus up to about 20 percent more Stick and band-pass centre at full velocity **[inference]**.
- **Loudness matching:** hats carry nearly all their energy above the snare's 1.5 kHz shelf, so the snare's weighting (+4 dB above 1.5 kHz) lifts every hat equally and mostly ranks them by plain RMS, which may over-rate bright hats; the spike should check by ear whether a steeper weighting is needed. Measure the closed hat and give the open hat the same gain, so the pair stays balanced **[inference]**.
- **First to cut** if fourteen is too many: Detune (lock at 0) and Comp. **First to add** if too narrow: a modal "Metal" layer (section 5) or FM pairs in place of the squares.

**MSP sketch:** six `rect~` (frequencies from the js, scaled by Pitch and Detune) into `+~`; `noise~`; an equal-power pair of `*~`; `svf~` (band output) then `svf~` (high output); `*~` by one `line~` envelope; stick from `noise~` into a fixed `svf~` high-pass with its own short `line~`; `degrade~`; then the shared Comp, Sat and Tone chain. The `PARTS` would be `"statics"`, `"amp"`, `"stick"`; closed and open differ only in which decay the voice's `ampPoints` uses **[inference]**.

## Open questions

1. **The band-pass and high-pass values of the real 808 hat path** were not reached this session. With the ICMC paper or the service manual open, the Colour and Thin ranges could centre on them.
2. **Does Detune earn its place,** or does Pitch alone give enough metal variety?
3. **Is the noise layer still a hat at full noise?** If not, cap Balance at about 0.8.
4. **Pedal note:** a third articulation, or not?
5. **Loudness:** match closed and open separately, or closed only with the open hat sharing its gain?

## Sources

- TR-808 circuit: <https://speech.di.uoa.gr/ICMC-SMC-2014/images/VOL_2/1453.pdf> (Werner, Abel and Smith, ICMC 2014; read via search only), <https://www.kvraudio.com/forum/viewtopic.php?p=7494787>, <https://www.elektronauts.com/t/hh-lab-tips/207960>, <https://e-licktronic.com/forum/viewtopic.php?p=958>, <https://www.manualslib.com/manual/1294765/Roland-Tr-808.html?page=6>, <https://support.roland.com/hc/en-us/articles/201963539-TR-808-Technical-Specifications>, <https://www.kraftmusic.com/tiptop-audio-hats808-analog-hi-hats-drum-module.html>, <https://docs.cherryaudio.com/misfit-audio/transistor-808-hats>
- Synth Secrets (Gordon Reid, Sound On Sound; via search only): <https://www.soundonsound.com/techniques/analysing-metallic-percussion>, <https://www.soundonsound.com/techniques/synthesizing-realistic-cymbals>, <https://www.soundonsound.com/node/4918183>
- TR-909: <https://en.wikipedia.org/wiki/Roland_TR-909>, <https://attackmagazine.com/?p=133892>
- Noise and FM hats: <https://noiseengineering.us/blogs/loquelic-literitas-the-blog/hats-off-to-this-series-synthesizing-hats-in-the-final-percussion-synthesis-post/>, <https://noiseengineering.us/blogs/loquelic-literitas-the-blog/creating-fm-hats/>, <https://www.musicradar.com/tuition/tech/make-noise-based-hi-hats-in-ableton-572822>, <https://musictech.net/tutorials/weekend-workshop-subtractive-drum-synthesis-tal-noisemaker/>, <https://docs.cycling74.com/api/latest/max8/tutorials/06_synthesischapter05>
- Ring-mod: <https://wikipedia.com/wiki/Ring_modulator>, <https://manuals.online/corsynth/synthesizer/dr-03-hi-hats-metal/user-manual/1g4yg>
- Modal and physical: <https://caml.music.mcgill.ca/~gary/618/week12/phism.html>, <https://music.arts.uci.edu/dobrian/maxcookbook/rhythmic-filtered-noise-bursts>, <https://cycling74.com/ja/forums/problem-with-changing-mcreson-frequency-steep-q-or-cymbal-sounds>, <https://www2.ph.ed.ac.uk/~sbilbao/platepage.html>, <https://perso.ensta-paris.fr/~touze/PDF/ducceschi_smac2013.pdf>
- Lo-fi and boom bap: <https://en.wikipedia.org/wiki/E-mu_SP-1200>, <https://www.rossum-electro.com/pages/education-sp-1200>, <https://www.musicradar.com/news/reissue-of-the-classic-sp-1200-sampler-announced-by-dave-rossum>, <https://www.native-instruments.com/blogs/blog/hip-hop-drums-101>, <https://www.attackmagazine.com/technique/beat-dissected/90s-boom-bap-hip-hop/>, <https://www.audeobox.com/learn/mpc-software/mpc-drum-programming/>, <https://www.musicradar.com/how-to/how-to-program-an-old-school-hip-hop-beat-in-your-daw>
- Max objects: <https://docs.cycling74.com/max8/refpages/rect~>, <https://docs.cycling74.com/max8/refpages/degrade~>, <https://cycling74.com/forums/degrade-bit-range>
