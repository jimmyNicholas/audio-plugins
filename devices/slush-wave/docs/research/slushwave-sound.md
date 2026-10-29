# Research: what makes the slushwave sound

Researched 2026-10-28 for ticket https://github.com/jimmyNicholas/audio-plugins/issues/2. Target: a Max for Live audio effect for Ableton Live 11 (Max 8).

## How to read this

- **Method caveat:** the sandbox could not fetch pages directly (DNS blocked for `en.wikipedia.org`, `microgenremusic.com` and others). Every claim below comes from search-engine extracts of the cited pages, not a full read. They are marked **[extract]**. Arithmetic or design inference by the researcher is marked **[inference]**.
- **Source quality:** there is no first-party technical write-up of slushwave. The genre's founder, t e l e p a t h (Luke Laurila), is famously evasive about method. So the "primary" sources here are genre definitions (Rate Your Music, Album of the Year, Last.fm), the founder's interview and press, and general vaporwave and slowed + reverb tutorials. Numbers come from tutorials and differ between them; treat them as ranges to expose on knobs, not as a spec.
- **Confidence:** High = several independent sources agree. Medium = one or two tutorials say it. Low = inference only.

## TL;DR

| Stage | Essential? | Typical settings | Confidence |
|-------|-----------|------------------|------------|
| Slowdown with pitch drop (varispeed, pitch and tempo together) | Essential | 0.65x to 0.85x speed (about -3 to -7.5 semitones); extreme tracks go far slower | High (stage) / Medium (numbers) |
| Reverb | Essential, the defining effect | Hall, plate or "cathedral"; long tail (3 to 6 s); around 40 to 60% wet; large room size 75 to 85% | High (stage) / Medium (numbers) |
| Phaser | Essential for slushwave specifically (genre is also called "phaserwave") | No published numbers; slow sweep is the inferred norm | High (stage) / Low (numbers) |
| Delay | Strongly typical | No published numbers; "gentle" | High (stage) / Low (numbers) |
| Chorus, flanger, tape wobble | Optional, light | Chorus about 30% (above 60% becomes an obvious wobble); wow around 0.5 Hz, flutter around 6 Hz and up, shallow | Medium |
| Low-pass / band-limiting filter | Optional but common | Low-pass 8 to 10 kHz (subtle) down to 6 kHz (heavy); high-pass around 100 Hz | Medium |
| Saturation, tape, noise, crackle | Optional | Soft tape or tube saturation, light noise bed | Medium |
| Looping a short sample section, very long track length | Structural, not an effect | Loops repeat for long tracks, sometimes 10+ minutes | High |

Suggested chain for the device: **slowdown/pitch drop -> filter -> phaser (and optional chorus/wobble) -> saturation -> delay -> reverb**, with saturation-after-reverb as a documented alternative.

## 1. What slushwave is (and why it differs from plain vaporwave)

- Album of the Year: a "Vaporwave subgenre that manipulates samples in a specific way to 'slush' them by slowing them down and adding reverb, delay and modulation effects, such as phasers." The name "was coined in 2013 by one of its most prominent artists, t e l e p a t h テレパシー能力者". **[extract]** https://www.albumoftheyear.org/genre/653-slushwave/
- Rate Your Music: the "slushy" sound is "achieved via the multi-layering of effects such as reverbs, delays, flangers, filters and especially phasers"; the genre is sometimes called "phaserwave". **[extract]** https://rateyourmusic.com/genre/slushwave/
- Last.fm: "slowed-down samples and reverb to create trance-like, hypnotic vaporwave." **[extract]** https://last.fm/tag/slushwave/wiki
- Micro Genre Music: slushwave extends samples to their limits by layering effects rather than chopping and rearranging them; "songs can be slowed down to 10% or even 5% of their original speed"; tracks are long and ambient in texture. **[extract]** https://microgenremusic.com/articles/what-is-slushwave/
- Sources: samples are often J-pop, City Pop, Cantopop and pop ballads. **[extract]** https://en.wikipedia.org/wiki/Telepath_(musician), https://vanpaugam.com/blog/what-is-slushwave-2023

**Takeaway [inference]:** the difference from classic vaporwave is not a different effect set but more of it: slower, wetter, and with modulation (phaser above all) layered on top, with little chopping.

## 2. Producer evidence: t e l e p a t h

- Sonemic (Rate Your Music) interview with Luke Laurila, 19 December 2025, by Nick Caceres. It describes his sound as "dappled in a haze of reverb, delays, and phasers". Asked about his tools he said it is "the same software, with some new additions resulting from my time away", without naming it. The interviewer calls the answers "abstruse, evasive". **[extract]** https://rateyourmusic.com/feature/sonemic-interview-luke-laurila-t-e-l-e-p-a-t-h-%E3%83%86%E3%83%AC%E3%83%91%E3%82%B7%E3%83%BC%E8%83%BD%E5%8A%9B%E8%80%85-starlight-temple
- Atwood Magazine and Wikipedia: his work is characterised by "constant reverberation and phaser effects, slow-paced melodies, and constant sample looping", mostly sampling J-pop from his own disc collection. Laurila named "Betrayal" (2013 split *Nightlife*) as his first definitive slushwave track. **[extract]** https://atwoodmagazine.com/telepath-t-e-l-e-p-a-t-h-music-feature/, https://en.wikipedia.org/wiki/Telepath_(musician)
- Listencorp on desert sand feels warm at night (a later slushwave act): the sound is typically "lacquered, in phasers and delay". **[extract]** https://listencorp.co.uk/post/mirage-music-exploring-the-work-of-desert-sand-feels-warm-at-night

**Gap:** no producer has published settings. Nothing found names his exact plug-ins, rates or decay times.

## 3. Stage by stage

### 3.1 Slowdown and pitch drop (essential)

- It is a varispeed (tape or turntable style) change: pitch and tempo fall together. In Audacity this is "Change Speed", not "Change Tempo". **[extract]** https://www.thewindowsclub.com/how-to-make-slowed-and-reverb-songs; in Ableton this is Re-Pitch warp mode, which "adjusts the playback rate" like a variable-speed turntable. **[extract]** https://pcaudiolabs.com/ableton-live-warping-part-6-how-to-use-re-pitch-warp-mode-in-ableton-live/
- Typical amounts, which differ by source **[extract]**:
  - slowed by 20 to 40%: https://bvker.com/how-to-vaporwave/ (search extract; attribution among the vaporwave guides is approximate)
  - about 65% speed as the "genre standard": https://futureproofmusicschool.com/blog/techniques-for-how-to-make-vaporwave-music-with-authentic-style
  - 70 to 85% of original speed: https://tunaryinstruments.com/guides/how-to-make-vaporwave/
  - pitch down 3 to 7 or 4 to 8 semitones: https://blog.samplefocus.com/blog/make-vaporwave-music-with-samples/, https://beatkey.app/how-to-make-vaporwave-music
  - slowed + reverb starting point 0.80x: https://nameonthebeat.com/en-us/blogs/alles-over-mixing/how-to-make-slowed-and-reverb
  - slushwave extremes 5 to 10% speed: Micro Genre Music (above).
- Ratio to semitones is `12 * log2(ratio)` **[inference]**: 0.85x = -2.8 st, 0.80x = -3.9 st, 0.75x = -5.0 st, 0.70x = -6.2 st, 0.65x = -7.5 st, 0.50x = -12 st. Note one guide (Beatkey) says a 25% slowdown drops "approximately 3-4 semitones"; the correct figure is about 5. Use the formula.
- Related lineage: chopped and screwed (DJ Screw) slowing; and Paulstretch extreme stretching, made famous by Nick Pittsinger's "U Smile 800% Slower" (2010), which kept pitch and sounded ambient. That is a different (pitch-preserving) process; the slushwave "slush" sources describe slowing with pitch drop. **[extract]** https://www.npr.org/sections/therecord/2010/08/18/129283985/the-art-of-a-time-stretch, https://en.wikipedia.org/wiki/Chopped_and_screwed

**Device design note [inference, important]:** a real-time audio effect cannot play its input slower than it arrives for ever; it would fall behind without limit. Options for the device: (a) pitch-shift down only and leave the slowdown to the clip (Re-Pitch warp or lowering song tempo), (b) record into a buffer and play back a loop at a slower rate (fits slushwave's looping), or (c) granular stretch. Option (b) is the closest to "slowed with pitch drop" inside a single effect; option (a) is the simplest.

### 3.2 Reverb (essential, the core of the sound)

- "Cathedral or tunnel" reverb: https://bvker.com/how-to-vaporwave/ ; "hall and plate reverbs with a long tail": https://futureproofmusicschool.com/blog/techniques-for-how-to-make-vaporwave-music-with-authentic-style ; "around 60% wet": vaporwave guide extract. **[extract]**
- Hall or plate, **3 to 6 s decay**: https://beatkey.app/how-to-make-vaporwave-music **[extract]**
- Slowed + reverb Audacity settings: Room Size 75 to 85%, Reverberance 40 to 60%, Damping 50%, Wet Gain -1 to -3 dB; dry gain below 0 dB to avoid clipping. Reverb about 40 to 50% with 0.80x speed as a classic start. **[extract]** https://nameonthebeat.com/en-us/blogs/alles-over-mixing/how-to-make-slowed-and-reverb, https://manual.audacityteam.org/man/reverb.html
- Damping makes the tail darker (highs decay faster). **[extract]** Audacity manual, above.
- Warning: too much reverb turns muddy. **[extract]** https://soniclab.io/blog/how-to-make-slowed-reverb
- Melodigging: "Drench elements in long reverb and gentle delay". **[extract]** https://www.melodigging.com/genre/slushwave

For slushwave specifically, the genre descriptions imply the wet end of these ranges (long decay, high mix) **[inference]**.

### 3.3 Phaser (essential for slushwave)

- The most characteristic modulation per Rate Your Music ("especially phasers", "phaserwave") and every telepath description (above). **[extract]**
- **No source gives phaser settings.** Suggested defaults, by inference only: slow LFO (about 0.05 to 0.5 Hz), moderate to high depth, moderate feedback, 50% mix or less so it reads as a slow "whoosh" through the reverb. **[inference, Low]**

### 3.4 Delay (strongly typical)

- Named in every slushwave definition (AOTY, RYM, Sonemic, Listencorp, Melodigging) as "delay" or "gentle delay". **[extract]**
- No settings found. A tempo-synced or long (300 to 700 ms), low-feedback, darkened delay feeding the reverb would match "gentle". **[inference, Low]**

### 3.5 Chorus, flanger and tape wobble (optional)

- Chorus is used lightly: "at 30% it is subtle; above 60% it becomes an obvious wobble"; "don't overdo it". **[extract]** vaporwave guides (search extract across https://futureproofmusicschool.com/blog/techniques-for-how-to-make-vaporwave-music-with-authentic-style and https://emastered.com/el/blog/how-to-make-vaporwave-music)
- Flangers listed by RYM among slushwave's layered effects. **[extract]**
- Tape wow and flutter: wow is the slow component below about 6 Hz, flutter is faster above about 6 Hz, with smaller amplitude. **[extract]** https://www.image-line.com/fl-studio-learning-content/fl-studio-mobile-online-manual/html/plugins/FL%20Studio%20Mobile_Module_Wow_and_Flutter.htm . Goodhertz Wow Control defaults to a 0.55 Hz rate (range 0.1 to 20 Hz). **[extract]** https://manuals.goodhertz.com/3.7/wow-ctrl/
- Suggested device range **[inference]**: wow 0.3 to 1 Hz, shallow depth (a few cents up to about 20 cents); flutter 5 to 12 Hz at lower depth.

### 3.6 Filtering (optional but common)

- Low-pass at 10 kHz for subtle, 6 kHz for heavy; roll off bass below 100 Hz and treble above 8 kHz. **[extract]** lo-fi and vaporwave guides (search extract including https://karanyisounds.com/blogs/production-tips/vaporwave-101-how-to-make-chillwave-music and https://beatkey.app/how-to-make-vaporwave-music)
- Melodigging suggests "slow filter modulation". RYM lists filters among the layered effects. **[extract]**
- Note: slowing already moves content downward, so the source's top octave is lost or dulled anyway. **[inference]**

### 3.7 Saturation, lo-fi, noise (optional)

- "Tape saturation, vinyl crackle, and light noise beds to add warmth and patina". **[extract]** https://www.melodigging.com/genre/slushwave
- Soft tube or tape saturation after a 3 to 6 s reverb in one lo-fi chain. **[extract]** https://beatkey.app/how-to-make-vaporwave-music
- VHS-style and tape artefacts are general vaporwave texture. **[extract]** vaporwave guides above.

### 3.8 Structure (not an effect, but part of the sound)

- Constant looping of a short sample section, long track lengths (often over ten minutes), ambient texture. **[extract]** Atwood, Micro Genre Music, vaporwave.wiki search extracts above.
- A buffer-loop slowdown (option (b) in 3.1) supports this directly. **[inference]**

## 4. Chain order

- General guidance: modulation (chorus, flanger, phaser) before time-based effects (delay, reverb) at the end. **[extract]** https://www.strymon.net/setting-up-your-effect-signal-chain/
- Saturation, then modulation, then delay/reverb. **[extract]** https://www.sonarworks.com/blog/learn/how-to-chain-multiple-vocal-effects-plugins-effectively
- Counterpoint: reverb before saturation in a lo-fi chain (Beatkey), and saturation after reverb "can result in interesting sound" if clipping is soft. **[extract]** https://beatkey.app/how-to-make-vaporwave-music, https://modwiggler.com/forum/viewtopic.php?t=261750
- The slowdown must come first: it is the transformation of the source, and every slowed + reverb guide does it before adding reverb. **[extract]** slowed + reverb guides above.

Recommended order:

1. Slowdown / pitch drop (varispeed or buffer loop)
2. Filter (high-pass about 100 Hz, low-pass 6 to 10 kHz, optional slow modulation)
3. Phaser (plus optional chorus or wow/flutter)
4. Saturation (or move to after reverb as a switch)
5. Delay
6. Reverb (long, wet)
7. Optional noise bed / crackle mixed in at the end

## 5. Essential versus optional

- **Essential (without these it is not slushwave):** slowdown with pitch drop, long wet reverb, phaser. Delay is near-essential (in every definition).
- **Optional colour:** chorus, flanger, tape wow/flutter, filtering, saturation, noise and crackle.
- Support: AOTY, RYM, Last.fm, Sonemic and Listencorp all name slowing plus reverb; most name delay and phaser; only tutorials and genre-tip sites add filter, saturation and noise. **[extract]**

## 6. Live 11 / Max 8 building blocks (pointer only, not researched in depth)

**[inference]** Live 11 stock devices cover most stages (Reverb, Hybrid Reverb, Phaser, Chorus-Ensemble, Flanger, Auto Filter, Echo, Saturator, Redux, Vinyl Distortion), so a prototype rack can test settings before writing a device. In Max 8, likely pieces are `buffer~`/`groove~` or `play~` for a slowed loop, `gen~` for varispeed, phaser and wow/flutter, `svf~`/`lores~` for filtering, `tapin~`/`tapout~` for delay, and a feedback-delay-network reverb in `gen~` (or `yafr2~`/`cverb~` style abstractions). Verify against the Max 8 docs before relying on these.

## Open questions

- Actual telepath settings and software: not published.
- Phaser and delay numbers: none found; will need ear-testing.
- Whether slushwave producers use pitch-preserving stretch (Paulstretch-style) for the extreme 5 to 10% cases or plain varispeed: sources do not say.
