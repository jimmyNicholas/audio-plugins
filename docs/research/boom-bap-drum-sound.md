# Research: what makes boom bap and experimental hip hop drums sound the way they do

Researched 2026-11-12 for ticket https://github.com/jimmyNicholas/audio-plugins/issues/31, part of the map https://github.com/jimmyNicholas/audio-plugins/issues/29 ("Map: Bloodline Breaks"). The album is sample-free: every drum comes from a breeder device (Kick, Snare with claps, Hats, 808). Boom bap is the core; Madlib, J Dilla, clipping. and Death Grips mark the experimental edge. Targets: Max for Live on Ableton Live 11 (Max 8, plain MSP, no `gen~`), and possibly C++ (JUCE) later.

The question: what gives these drums their sound, in terms a synthesis engine can reach without samples, and how cheaply?

## How to read this

- **Method caveat:** direct page fetches failed in this sandbox (`www.rossum-electro.com` and `www.muzines.co.uk` did not resolve, so per the session's network rules no further sites were tried). Every web claim comes from search-engine extracts of the cited pages, not a full read, and is marked **[extract]**. Claims read from this repository's own code are marked **[read]**. The researcher's arithmetic, design reasoning and suggested numbers are marked **[inference]**. Facts brought in from general knowledge with no fetched source are marked **[background, unverified]**.
- **Source quality:** the strongest sources here are the maker's own pages (Rossum Electro-Music, who reissued the SP-1200 with E-mu's original designer Dave Rossum), the Max reference pages from Cycling '74, and contemporary reviews. A lot of the sampler lore (filters per output, the 45 rpm trick, which machine is "grittier") lives only in forums. Those lines say so. Producers' own drum settings are almost never published; no source was found giving a named producer's tuning in Hz.
- **Confidence:** High = a maker's page or several independent sources agree. Medium = one or two sources, or forum agreement. Low = inference or a single forum post.
- **Scope:** sound only. Pattern and timing (swing, Dilla's off-grid feel) belong to the album's sequencing and are touched on only in section 6.
- **Related notes:** the slushwave note covers tape wobble and low-pass settings for a whole mix: [slushwave-sound.md](../../devices/slush-wave/docs/research/slushwave-sound.md). The current voices are specified in [Kick Breeder](../../devices/kick-breeder/docs/SPEC.md) and [Snare Breeder](../../devices/snare-breeder/docs/SPEC.md).

## TL;DR

| # | Finding | Confidence |
|---|---------|------------|
| 1 | The boom bap sound is mostly **the sampler, not the drum**: the E-mu SP-1200 (26.04 kHz, 12-bit linear, drop-sample pitch shifting with no interpolation, SSM2044 analogue low-pass filters) and the Akai MPC60 (40 kHz, 12-bit non-linear, about 18 kHz bandwidth). | High (specs) |
| 2 | The **SP-1200 crunch comes mainly from aliasing and imaging**, made worse by sampling records fast (33 at 45 rpm) and pitching down. 12-bit quantisation alone is subtle: forum users call the MPC60's 12-bit sound "subtle" next to the SP's. | High (mechanism) / Medium (relative weight) |
| 3 | Grit only bites where there is high-frequency content to fold. A synthesised sine kick barely changes under sample-rate reduction; clicks, snare wires, claps and hats change a lot. So **drive before the crusher**, as Attack Magazine advises. | Medium (Attack) / High (inference from sampling theory) |
| 4 | Drums in boom bap come from **breaks** (Amen, Funky Drummer, Funky President, Impeach the President): acoustic kicks with short decays, snappy snares, open hats, often layered or replaced hit by hit. A synthesised boom bap kick wants **less pitch sweep and shorter decay** than an electronic kick, plus a room or noise layer. | Medium |
| 5 | The **808** in hip hop is a tuned bass: a bridged-T oscillator with long decay and a slight pitch drop in the tail. Rick Rubin and Original Concept popularised long, retuned 808 kicks; trap made it a pitched bass with glide and distortion. | Medium |
| 6 | **Vinyl character** for a synth engine is three cheap layers: hiss (high-passed noise), crackle (random impulses, about 5 to 15 per second, band-passed 0.8 to 8 kHz) and pops (about 0.1 to 2 per second), plus the SP-303 "Vinyl Sim" style compression Madlib is known for. | Medium |
| 7 | The **experimental end** pushes the same knobs past taste: clipping. builds tracks with no drums or from white-noise volleys; Death Grips had Zach Hill trigger percussion samples by hand, with heavy crunch and distortion. For a breeder this means wider ranges (hard clip, 4 to 8 bits, very low sample rates, noise-only "drums"), not new modules. | Medium |
| 8 | Everything listed is **cheap in both C++ and plain MSP**. `degrade~` already does SP-style sample-rate and bit reduction with no interpolation and no dither, which is exactly the vintage behaviour. Only tape wow on a one-shot needs a delay line, and per-hit pitch drift in the `js` is a cheaper stand-in. | High (Max docs) |

Suggested ranges per drum are in section 8.

## 1. The samplers and their artefacts

### 1.1 E-mu SP-1200 (1987)

- **Format:** "12-bit linear data format and the same 26.04 kHz sample rate E-mu previously used in Drumulator and SP-12". Rossum's reissue keeps "exactly the same 12-bit linear data format and 26.04 kHz sampling rate", and its product page says the original's "audible aliasing and imaging artifacts are preserved identically". **[extract]** https://en.wikipedia.org/wiki/E-mu_SP-1200 ; https://www.rossum-electro.com/pages/education-sp-1200 ; https://www.rossum-electro.com/products/sp-1200/ . **Confidence:** High.
- **Memory:** about 10 seconds in total, with a maximum single sample of 2.5 seconds. **[extract]** https://en.wikipedia.org/wiki/E-mu_SP-1200 . **Confidence:** High.
- **Pitch shifting:** playback uses drop-sample pitch shifting, which "produces significant additional, unfiltered audible artifacts and distortion". A Gearspace taxonomy of early digital instruments lists the SP-12 (about 26 kHz) among drop-sample (nearest-neighbour) machines, which keep the image frequencies and so alias. **[extract]** Wikipedia via mirror https://cavac.at/cavacopedia/E-mu%20SP-1200 ; https://gearspace.com/board/electronic-music-instruments-and-electronic-music-production/826280-taxonomy-early-digital-synthesizers.html . **Confidence:** Medium (no maker's description of the algorithm was found).
- **The 45 rpm trick:** to beat the short memory, producers sampled 33 rpm records at 45 rpm (plus extra pitch up), then tuned the sample back down on playback. This "expanded the total sampling time while at the same time decreasing the resolution". Forum users say pitching up then down makes the artefacts "more present"; at least one says they use the trick on loops but sample drum hits normally. **[extract]** https://en.wikipedia.org/wiki/E-mu_SP-1200 ; https://www.tapatalk.com/groups/sp1200/sp-sound-and-sampling-techniques-t827.html ; https://www.mpc-forums.com/viewtopic.php?p=1063109 . **Confidence:** High (practice) / Medium (how much drums used it).
- **What the trick does in numbers [inference]:** sampling at 45 instead of 33.3 rpm raises the pitch by 45 / 33.3 = 1.35 (about +5.2 semitones). Playing back 5.2 semitones down stretches each stored sample, so the effective sample rate of the result is 26.04 / 1.35 = about 19.3 kHz, and the effective bandwidth falls from about 13 kHz to about 9.6 kHz, with the drop-sample images now falling lower and louder. That is the "pitched-down SP" sound.
- **Filters:** the SP-1200 uses the analogue SSM2044 4-pole low-pass chip; running out of those chips ended production in 1998. Rossum's reissue has cutoff and resonance sliders for channels 1 and 2. Forum measurements describe outputs 1 and 2 as a "dynamic" filter that behaves like an attack and decay envelope, and outputs 3 to 8 as fixed low-pass filters with less roll-off on higher pairs (one reader estimates around 6 kHz); one poster "thought" 7 and 8 were unfiltered but had not checked. **[extract]** https://en.wikipedia.org/wiki/E-mu_SP-1200 ; https://www.tapatalk.com/groups/sp1200/backlight-t673-s10.html ; https://www.mpc-forums.com/viewtopic.php?p=1549820 . **Confidence:** High (SSM2044) / Low (per-output cutoffs).
- **Tune or decay, not both:** the manual (quoted on the SP-1200 forum) says each sound has its slider set to vary tuning or decay "but not both"; drums and claps default to tuning, cymbals to decay. Tuning a cymbal gives "more of a ring modulation type of effect". **[extract]** https://www.tapatalk.com/groups/sp1200/tune-decay-t1946.html . **Confidence:** Medium. **Takeaway [inference]:** the ring-mod sound on pitched cymbals is the drop-sample images; a synthesised hat run through a sample-and-hold at a low rate gets the same metallic sidebands.
- **Who:** Pete Rock: "everything that you ever heard from me back in the day was the SP-1200". **[extract]** via https://gearspace.com/threads/rzas-drums.1419090/ (second-hand quote). **Confidence:** Medium.

### 1.2 Akai MPC60 (1988)

- **Format:** co-designed by Roger Linn and Akai. 12-bit resolution in a "non-linear format for reduced noise", sample rate "fixed at 40kHz (giving an 18kHz frequency response)", per a contemporary review. MusicRadar says the non-linear format "rolled off a little of the highs in a very pleasing way". Sample time is given as 13.1 or 13.7 seconds depending on source; one CDM piece says 14-bit, against the usual 12-bit. **[extract]** https://www.muzines.co.uk/articles/akai-mpc60/1079 ; https://www.musicradar.com/news/vintage-music-tech-icons-akai-mpc3000 ; https://cdm.link/?p=164766 . **Confidence:** High (40 kHz, 12-bit non-linear) / Low (memory, 14-bit).
- **Compared with the SP-1200:** forum users say the MPC60's 12-bit sound is "subtle" and the SP's is grainier, especially when pitched down; one says SP drums sound punchier, the MPC3000 more "buttery". **[extract]** https://www.mpc-forums.com/viewtopic.php?p=200236 and thread. **Confidence:** Low to Medium (opinion, but consistent).
- **What "non-linear 12-bit" means [inference]:** a companding curve (like mu-law) spends more of the 4,096 steps near zero. Quiet tails stay clean and loud transients take the coarser steps, so the MPC60 hides quantisation in decays where the SP-1200's linear 12 bits would get grainy. Treat the curve as unknown; mu-law with mu around 15 to 255 is a reasonable stand-in.
- **Timing:** Roger Linn says even skilled players struggle to play 58 to 62% swing live without quantisation; the MPC swing settings are central to the "MPC feel". **[extract]** https://www.attackmagazine.com/features/interview/roger-linn-swing-groove-magic-mpc-timing/3/ . Out of scope for the voice; see section 6.

### 1.3 Boss SP-303 and Madlib

- The SP-303 "Vinyl Sim" effect (carried into the SP-404MKII as "303 VinylSim") recreates "the warm, idiosyncratic sound of a spinning record". Its signature is a compressor that Goodhertz modelled as Vulf Compressor; one profile says it "simultaneously exaggerates the dust, crackle, and imperfections in the record". *Madvillainy* was made on an SP-303. **[extract]** https://static.roland.com/manuals/sp-404mk2_reference_v4/en-US/7984244378561675.html ; https://goodhertz.com/tonal/vinyl-sim ; https://create.routenote.com/blog/producer-profile-madlib/ . **Confidence:** Medium. No published numbers for the Vinyl Sim compressor were found.
- **Takeaway [inference]:** the Madlib sound is a heavy, slow-release, pumping compression over the whole kit plus surface noise brought up by it. On a one-shot voice that is the existing envelope compression (tail pulled up against the hit) pushed hard, plus a noise floor that also rises in the tail.

### 1.4 Turning the sampler into "grit" for a synthesised voice

| Artefact | What it is | Synthesis stand-in | Confidence |
|---|---|---|---|
| Low sample rate | Bandwidth capped at half the rate (13 kHz at 26.04 kHz) | Sample-and-hold at a lower rate, then optionally a low-pass | High |
| Aliasing and imaging | No interpolation on pitch change, so content folds back and steps leave images | The same sample-and-hold with **no** anti-alias filter in front; brighter input folds more | High |
| Pitched-down sample | 45 rpm trick: effective rate drops to about 19 kHz | Lower the hold rate further (ratio about 0.44 of 44.1 kHz) | Medium |
| 12-bit linear | About 74 dB dynamic range (6.02 x 12 + 1.76) **[inference]** | Round to 12 bits; audible mainly in quiet tails | High |
| 12-bit non-linear (MPC60) | Companded steps, cleaner tails | Compress with a mu-law curve, round, expand | Low (curve unknown) |
| SSM2044 filter | 4-pole resonant low-pass, envelope-swept on SP channels 1 and 2 | 4-pole low-pass (two cascaded 2-pole), cutoff with its own short decay | Medium |
| Fixed output filter | Gentle low-pass on other outputs | One-pole or 2-pole low-pass at 6 to 12 kHz | Low |
| Truncation | Short memory forced cut-off tails | A gate or fast release at a set length | Medium |

Attack Magazine's advice for the same job: combine bit crushing, sample-rate reduction, saturation, filtering and EQ, put analogue-style saturation **before** the crusher, and pitch sounds down to bring out low-resolution character. A Loopcloud guide suggests going past the SP for drums, to about 21 kHz and 11 bits. **[extract]** https://www.attackmagazine.com/technique/character-crunch-replicating-the-sound-of-vintage-samplers/ ; https://loopcloud.com/cloud/blog/5185-Unlocking-Vintage-Vibes-Mastering-Classic-Sampler-Techniques-in-Modern-Music-Production . **Confidence:** Medium.

**Why order matters for a synth voice [inference]:** a sampler crushes a recording that already has cymbal wash, room and noise up to 20 kHz. A synthesised kick is nearly a pure sine, so holding it at 26 kHz changes almost nothing. To get crunch, the engine must first make content above the new Nyquist (drive, click, noise), then crush it. So the chain is voice, drive, crusher, filter, and the drive amount should breed together with the crusher rate.

## 2. Vinyl and tape character

- **Vinyl surface noise as a model:** one documented open-source model uses three layers: hiss (high-passed at 300 Hz), crackle (0 to 200 events per second, default 15, band-passed to 800 to 8,000 Hz) and pops (0 to 20 per second, default 1.5). Each sample triggers a crackle with probability rate / sample rate (a Poisson process). A SuperCollider patch uses about 8 impulses per second for soft crackle, faster modulated impulses for pops, and brown noise through a 1.2 kHz low-pass for rumble. **[extract]** https://foliodsp.pages.dev/components/color/vinyl-noise ; https://sccode.org/1-4Sj . **Confidence:** Medium (two independent implementations agree on shape).
- Moliner and Välimäki (DAFx 2022) note that clicks synthesised from random numbers with set duration, amplitude and rate "tend to sound unrealistic", and propose a diffusion model instead. **[extract]** https://arxiv.org/pdf/2206.06259v2.pdf . **Takeaway [inference]:** for drums the crackle sits under a loud hit, so a simple random model is enough; realism matters more for exposed loops.
- **Tape:** a tape machine rolls off highs and adds a low-frequency "head bump"; it saturates at high levels. Wow and flutter boundaries differ by source: under 4 Hz is wow in one, 0.1 to 10 Hz wow and 10 to 100 Hz flutter in Sound On Sound's. **[extract]** https://downloads.izotope.com/docs/ozone8/vintage-tape/index.html ; https://www.soundonsound.com/techniques/analogue-warmth ; https://sageaudio.com/articles/tape-emulation . **Confidence:** Medium.
- **On a one-shot [inference]:** wow is too slow to hear inside a 300 ms drum. Its useful effect is that each hit lands at a slightly different pitch. A random pitch offset per hit (a few cents) in the `js` gives that for free, with no delay line. Head bump is a gentle low shelf or peak around 50 to 100 Hz, which overlaps the kick's body; it can be left to the kick's own tuning.

## 3. The drums themselves

### 3.1 Where boom bap drums come from

- The most-sampled breaks: The Winstons' "Amen, Brother" (over 2,000 uses on WhoSampled per FiveThirtyEight), James Brown's "Funky Drummer" (1,302) and "Funky President" (736), and The Honey Drippers' "Impeach the President" (650), whose "boom-bap staples include skipping bass drums, open hi-hats and snappy snare drums". **[extract]** https://fivethirtyeight.com/features/the-most-sampled-song-of-all-time ; https://producelikeapro.com/blog/5-most-sampled-drum-beats-in-hip-hop/ . **Confidence:** High (rankings vary by source).
- DJ Premier, in a 2003 interview quoted on a forum, describes grabbing a single snare and kick from a record to build a new pattern. **[extract]** https://www.2pacboard.com/community/threads/2003-dj-premier-interview.9066 . **Confidence:** Low to Medium.
- Boom bap mixing favours "punchy, mid-forward drums over a clean, polished sound". **[extract]** https://violetrecording.com/how-to-make-boom-bap-beats/ . **Confidence:** Low (single guide).
- **What follows for synthesis [inference]:** these are acoustic drums, recorded in a room, mastered to vinyl, then sampled. So a boom bap kick is an acoustic kick: a low body with a modest pitch drop, a beater thump, room noise, and a short tail (the next hit or the truncation cuts it). The snare is a real snare: a body around 150 to 250 Hz, a lot of wire noise, and room. The hats are dark, washy and often open.

### 3.2 Kick

- Hip hop kicks usually have their sub around 40 to 60 Hz, with "punch" around 80 to 100 Hz. **[extract]** https://unison.audio/how-to-eq-drums/ . **Confidence:** Low to Medium (mixing guides, not measurement).
- **Shape [inference]:** fast attack (beater, 1 to 3 ms click in the 1 to 4 kHz range), body decay of 150 to 400 ms for a break kick, pitch sweep of a few semitones over 10 to 40 ms (acoustic heads drop less and faster than an electronic kick). A room layer (low-passed noise, 50 to 150 ms) makes it sound "sampled off a record". Layering a break-style kick over a long 808 is common in later boom bap (section 3.6).

### 3.3 Snare

- Snare energy runs roughly 200 Hz to 10 kHz in mixing guides; high-pass points of 80 to 200 Hz are given "depending on the fundamental". No source gave a target snare tuning in Hz for boom bap. **[extract]** https://unison.audio/how-to-eq-drums/ ; https://emastered.com/blog/how-to-eq-drums . **Confidence:** Low.
- **Shape [inference]:** for the "snappy" break snare: body 160 to 260 Hz with a short decay (60 to 200 ms), wires decay 120 to 350 ms, wire tone peaking 2 to 6 kHz (dulled by the sampler's filter, so Tone well under 10 kHz), plenty of Snap. Layering a clap or a second snare on the backbeat is a standard thickener (general tutorials say "add additional kick or snare samples for more punch" but not too many). **[extract]** for layering: https://blog.samplefocus.com/blog/dj-premier-breakbeat-hip-hop-samples . **Confidence:** Low.

### 3.4 Claps

- The TR-808 clap's service manual describes a "hand clap generating cycle" in which the output becomes "a train of narrow pulses", the source of the clap's repeated bursts; builders describe its envelope as a simple attack-release with retrigger. No reliable source gave burst spacing or the tail mechanism. **[extract]** https://www.manualslib.com/manual/1294765/Roland-Tr-808.html?page=6 ; https://www.kvraudio.com/forum/viewtopic.php?p=8606994 . **Confidence:** Medium (bursts) / Low (numbers).
- **Shape [inference, background unverified for the numbers]:** 3 or 4 bursts of band-passed noise about 7 to 15 ms apart, centred around 1 to 2.5 kHz, then a longer tail of 100 to 300 ms. Snare Breeder's Bursts and Spread (1 to 4, 4 to 20 ms) already cover this; for claps the useful corner is Bursts 3 to 4, Spread 6 to 14 ms, body low.

### 3.5 Hats

- **The 808 metal source:** six square-wave oscillators from one HD14584 Schmitt-trigger chip, nominally 205.3, 304.4, 369.6, 522.7 Hz plus two trimmed oscillators set to about 540 and 800 Hz (untrimmed ranges about 254 to 627 and 359 to 1,150 Hz). A synthesis primer says the spread "is not that critical", roughly 200 to 800 Hz. These six are band-passed and high-passed into the hats and cymbal. **[extract]** https://www.kvraudio.com/forum/viewtopic.php?p=7494787 ; https://speech.di.uoa.gr/ICMC-SMC-2014/images/VOL_2/1453.pdf ; https://docs.cherryaudio.com/misfit-audio/transistor-808-hats . **Confidence:** Medium (community figures consistent with an ICMC paper).
- **Shape [inference]:** boom bap hats come off breaks, so they are darker and longer than 808 or trap hats: closed 30 to 90 ms, open 200 to 800 ms, band centre 6 to 10 kHz but filtered down by the sampler to 8 to 12 kHz at most. Through a 26 kHz sample-and-hold, the square-wave metal and noise fold into the "ring modulation" sound the SP manual describes. Closed and open must choke (already noted in the Snare Breeder spec).

### 3.6 808s across boom bap and later hip hop

- **Circuit:** the 808 kick is a bridged-T resonator "kicked into action" by the trigger, with no VCA in the loop; decay is set by feedback, longer decays let the pitch drop slightly at the end of the tail, and the service manual notes the timbre changes over the decay. Tuning is set by a resistor and drifts with capacitor age. **[extract]** https://www.pressreader.com/australia/computer-music/20190220/282381220809560 ; https://www.manualslib.com/manual/1294765/Roland-Tr-808.html?page=6 ; https://e-licktronic.com/forum/viewtopic.php?p=8499 . **Confidence:** Medium.
- **In hip hop:** Rick Rubin and Original Concept are credited with lengthening the 808 kick's decay and tuning it to different pitches; the kick has been used as a bass line in hip hop "because of its tuning and long decay". **[extract]** https://en.wikipedia.org/wiki/Tadao_Kikumoto (and the TR-808 article it summarises). **Confidence:** Medium.
- **Later:** Kanye West's *808s & Heartbreak* (2008) is named after the machine; Atlanta's 808 Mafia are named after it; in trap the 808 is set to the song's key, played as a bass with glide or cut notes, and often distorted with saturation or soft clipping. **[extract]** https://www.slate.com/articles/arts/music_box/2008/12/whats_an_808.html ; https://www.thomannmusic.com/blog/bass-beats-and-legend-the-roland-tr%E2%80%91808-and-what-makes-it-so-iconic/ ; https://orphiq.com/resources/808-drums-explained . **Confidence:** High (history) / Low (trap technique, weak sources).
- **What follows [inference]:** an 808 device is a kick voice with three changes: pitch from MIDI note (Kick Breeder's proposed "Pitch from note" toggle), a much longer decay (0.4 to 3 s), and glide between overlapping notes (mono with portamento). Its sweep is small (a few semitones) and its tail drops slightly; distortion is where the breeding range should be widest, since a distorted 808 is how it reads on small speakers.

## 4. The experimental end

- **Madlib:** SP-303, Vinyl Sim compression and dust (section 1.3). **Confidence:** Medium.
- **J Dilla:** played the MPC unquantised; accounts describe kicks and snares a few tens of milliseconds off the grid with different swing per drum, and kicks and hats "bending" around the beat. **[extract]** https://www.musicradar.com/news/pioneers-j-dilla ; https://brettworks.com/2013/07/23/roger-linn-on-drum-machine-groove-and-j-dillas-off-beat-sound/ ; https://loudlands1.squarespace.com/blog/drumming-with-microtime-part-1 . **Confidence:** Medium. A sound matter only in that per-hit variation (velocity, tiny pitch and decay changes) makes off-grid hits sound played rather than programmed **[inference]**.
- **clipping.:** Jonathan Snipes describes a project rule of no drums and no pitched material that still had to feel rhythmic; "Body & Blood" and "Get Up" have no drums, the latter built round an alarm-clock recording; live sets are described as "volleys of white noise"; earlier beats were built from "crunchy breaks". **[extract]** https://www.perfectcircuit.com/signal/jonathan-snipes-interview ; https://lamniformes.substack.com/p/drumming-upstream-2-body-and-blood ; https://www.spinmagazine.com/2014/01/clipping-interview-noise-rap-sub-pop-midcity/ ; https://www.thelineofbestfit.com/features/interviews/nine-songs-clipping . **Confidence:** Medium.
- **Death Grips:** the band said Zach Hill would trigger the percussion samples on *No Love Deep Web* by hand rather than program them; Hill usually plays triggered electronics; critics describe "inherent crunchiness, distortion and glitchiness". **[extract]** https://talkhouse.com/hunter-hunt-hendrix-of-liturgy-and-survival-talks-death-grips-no-love-deep-web ; https://www.rollingstone.com/music/music-album-reviews/review-death-grips-bottomless-pit-sounds-like-no-punk-on-earth-122585/ ; https://www.jhunewsletter.com/article/2012/05/death-grips-the-internet-band-of-the-21st-century-19381 . **Confidence:** Medium (no production breakdown found).
- **What follows [inference]:** the experimental end does not need new modules. It needs the same knobs with wider ends: drive into hard clipping, bit depth down to 4 to 6, hold rates down to 2 to 6 kHz, noise-only "kicks" and "snares" (body level 0), and very long or very short decays. In a breeder, a separate "Wide" range set (or an Era macro, section 7) keeps random boom bap searches from landing in noise too often while still letting a search walk there.

## 5. Cost in C++ (JUCE) and plain MSP (Max 8, no `gen~`)

Max facts are from the Cycling '74 reference **[extract]**; the current spikes' object list is **[read]** from `shared/breeder/patch.mjs`, which already uses `cycle~`, `noise~`, `line~`, `svf~`, `lores~`, `tanh~` and `clip~`. JUCE class names are **[background, unverified]**.

| Grit stage | C++ (JUCE) | Plain MSP | Cost |
|---|---|---|---|
| Sample-rate reduction (drop-sample, no interpolation) | A phase counter and a held value: a few operations per sample | `degrade~` (first argument: ratio of new rate to system rate; "does not use any interpolation when resampling, nor any dithering when quantizing", intended for lo-fi). At 44.1 kHz, ratio 0.59 is SP (26.04 kHz), 0.91 is MPC60 (40 kHz) | Very cheap in both |
| Bit reduction (linear) | `round(x * 2^(b-1)) / 2^(b-1)` | `degrade~` second argument, 1 to 52 bits (docs warn low bit counts "can create very loud, noisy signals"); or `*~`, `round~`, `/~` | Very cheap |
| Companded 12-bit (MPC60 style) | mu-law compress, quantise, expand: a log, an exp and a round | `lookup~` with a curve in a `buffer~`, or chains of `pow~` and `log~` round `degrade~` | Cheap in C++, moderate patching in MSP |
| Saturation | `std::tanh` or `juce::dsp::WaveShaper` | `tanh~` (already used), `overdrive~` (waveshaper limited to +/-1), `clip~` for hard clip | Very cheap |
| Fold or asymmetric drive | A few lines of maths | `lookup~` transfer function (Max's distortion tutorial notes extra zero-crossings add harmonics) | Cheap |
| Resonant low-pass (SSM2044 stand-in) | `juce::dsp::LadderFilter` or two cascaded `StateVariableTPTFilter`s | `lores~` (2-pole, already used) or two in series for 4-pole; `svf~` (Chamberlin state-variable: low, high, band and notch at once) | Cheap |
| Swept output filter (SP channels 1 and 2) | Envelope on cutoff | `line~` into the filter's cutoff inlet | Cheap |
| Hiss and rumble | White noise into one-pole filters | `noise~` or `pink~` into `onepole~` or `svf~` | Very cheap |
| Crackle and pops | Per-sample random compare (Poisson), short decaying click through a band-pass | `noise~` into `>~` threshold gives sparse impulses; or `metro` with random period in the event world into `click~`, then `svf~` band-pass | Cheap |
| Per-hit pitch drift (wow stand-in) | Random offset at note-on | Random offset in the `js` before the `line~` breakpoints | Free |
| Real wow and flutter | Modulated delay line with interpolation | `tapin~` / `tapout~` with a modulated delay time (needs care to avoid zipper noise) | Moderate in both; not worth it on one-shots |
| Pitch-down trick (render high, play slow) | Render the hit to a buffer, play back slower with drop-sample reading | `buffer~`, `record~`, `play~` (or `groove~`) with interpolation off where possible | Moderate; a lower `degrade~` ratio gets most of the sound **[inference]** |
| Vinyl Sim style compression | Envelope follower and gain computer | The spikes' approach: compress the envelope breakpoints in the `js` (no audio-rate detector), plus raise the noise floor with the same curve | Free in the `js` |

**Gotchas [inference]:**
- `degrade~` without a filter after it leaves images up to the system Nyquist, which is the SP's character but also sounds "digital" on hats. Breed a post-crush low-pass (Tone) alongside the rate, not instead of it.
- Quantisation is level-dependent: a loud hit hides 12-bit steps; the tail exposes them. A one-shot fading into 8 bits turns into buzzing steps before silence, which is an effect worth breeding, and also a reason to compute the loudness match after the crusher.
- `degrade~` is per channel and stateless across notes, so polyphony is free in the device sense (one stream).

## 6. Pattern notes (out of scope, for the album)

- MPC swing in the 54 to 62% range is the classic feel; Linn says 58 to 62% is hard to play by hand. **[extract]** https://www.attackmagazine.com/features/interview/roger-linn-swing-groove-magic-mpc-timing/3/ ; https://www.mpc-forums.com/viewtopic.php?p=412257 .
- Dilla-style feel: different swing or offsets per drum (examples of 20 to 30 ms early or late), unquantised hits. **[extract]** https://www.musicradar.com/news/pioneers-j-dilla .
- No source confirmed a boom bap tempo range; 85 to 95 BPM is common lore **[background, unverified]**.

## 7. A shared grit stage: Era

**Proposal [inference]:** every breeder voice ends in the same grit chain, so the album's kit sounds like it went through one machine:

`voice -> drive (tanh~) -> crusher (degrade~: rate ratio, bits) -> filter (lores~, with optional swept cutoff) -> noise bed (hiss, crackle) -> out`

- An **Era** setting (0 to 1) can move several settings at once along a line from clean to SP pitched down: rate ratio 1.0 to 0.4, bits 16 to 10, filter 16 kHz to 6 kHz, noise -70 to -40 dB. Breeding then has one setting that means "how sampled does this sound", with the separate settings still lockable.
- Whether the grit chain breeds per drum or is set once for the kit is a design question for the map (per drum is more flexible; per kit is more coherent).

## 8. Suggested settings and ranges per drum

All ranges are **[inference]** from the sections above; none is a published preset. "Core" is where random boom bap candidates should start; "Wide" is how far a search may walk for the experimental end. Settings already in the spikes keep their names.

### 8.1 Kick (boom bap break kick)

| Setting | What it does | Core range (proposed) | Wide range (proposed) |
| --- | --- | --- | --- |
| Pitch | Where the body settles | 45 to 75 Hz | 30 to 120 Hz |
| Sweep | How far above Pitch the hit starts | 3 to 18 semitones | 0 to 48 semitones |
| Sweep time | How fast the pitch falls | 8 to 40 ms | 5 to 200 ms |
| Decay | Body length | 150 to 450 ms | 50 to 1500 ms |
| Click level | Beater thump | 0.2 to 0.7 | 0 to 1 |
| Click tone | Beater brightness | 1 to 5 kHz | 500 Hz to 12 kHz |
| Noise level, decay, tone | Room layer | 0.1 to 0.4, 40 to 150 ms, 300 Hz to 2 kHz | 0 to 1, 10 to 1000 ms, 200 Hz to 12 kHz |
| Compression | Tail pulled up against the hit | 0.3 to 0.7 | 0 to 1 |
| Saturation | Drive before the crusher | 0.2 to 0.6 | 0 to 1 |
| Tone | Low-pass over the kick | 3 to 9 kHz | 200 Hz to 16 kHz |

### 8.2 Snare (break snare)

| Part | Settings | Core range (proposed) | Wide range (proposed) |
| --- | --- | --- | --- |
| Skin | Pitch; Ratio, Ring; sweep; Body decay | 160 to 260 Hz; 1.4 to 1.8, 0.2 to 0.6; 0 to 8 semitones over 5 to 30 ms; 60 to 200 ms | As the current spec: 120 to 400 Hz; 1.3 to 2.4, 0 to 1; 0 to 24 semitones over 5 to 80 ms; 30 to 500 ms |
| Balance | Skin to wires | 0.45 to 0.75 (wire-heavy) | 0 to 1 |
| Wires | Decay, tone, width | 120 to 350 ms, 2 to 6 kHz, medium to wide | 40 to 800 ms, 800 Hz to 10 kHz, wide to narrow |
| Bursts | Hits before the tail | 1 (sometimes 2, 3 to 6 ms apart, for flam) | 1 to 4, 4 to 20 ms |
| Snap | Crack of high-passed noise | 0.3 to 0.8 | 0 to 1 |
| Comp, Sat, Tone | As on the kick | 0.3 to 0.7, 0.2 to 0.6, 4 to 9 kHz | 0 to 1, 0 to 1, 1 to 16 kHz |

### 8.3 Clap (Snare Breeder with Bursts, or its own device)

| Part | Settings | Core range (proposed) | Wide range (proposed) |
| --- | --- | --- | --- |
| Bursts | Number of claps before the tail | 3 to 4 | 1 to 6 |
| Spread | Gap between bursts | 6 to 14 ms | 3 to 30 ms |
| Noise band | Centre, width | 1 to 2.5 kHz, medium | 500 Hz to 6 kHz, wide to narrow |
| Tail | Decay after the last burst | 100 to 300 ms | 40 to 800 ms |
| Skin | Body level under the clap | 0 to 0.2 | 0 to 1 |
| Layer with snare | Clap level when stacked on a snare | 0.3 to 0.6 of snare level | Any |

### 8.4 Hats (closed and open, choking)

| Part | Settings | Core range (proposed) | Wide range (proposed) |
| --- | --- | --- | --- |
| Metal | Six squares, base frequency and spread (808 ratios: 205, 304, 370, 523, 540, 800 Hz) | Base 180 to 260 Hz, spread 0.8 to 1.2 of 808 ratios | Base 100 to 600 Hz, spread 0.5 to 2 |
| Metal to noise | Square bank against white noise | 0.3 to 0.7 | 0 to 1 |
| Band | Band-pass centre, high-pass | 6 to 10 kHz, 4 to 7 kHz | 2 to 14 kHz, 1 to 10 kHz |
| Closed decay | Length of closed hat | 30 to 90 ms | 10 to 200 ms |
| Open decay | Length of open hat | 200 to 800 ms | 100 to 2000 ms |
| Tone | Low-pass after the band | 7 to 12 kHz | 2 to 16 kHz |
| Choke | Closed cuts open | On | On or off |

### 8.5 808 (tuned bass kick)

| Setting | What it does | Core range (proposed) | Wide range (proposed) |
| --- | --- | --- | --- |
| Pitch | From MIDI note, plus fine tune | Notes giving 32 to 65 Hz (about C1 to C2), fine +/-50 cents | 25 to 130 Hz |
| Sweep | Start above Pitch | 0 to 7 semitones | 0 to 24 semitones |
| Sweep time | Fall time | 10 to 60 ms | 5 to 200 ms |
| Tail drop | Slight pitch fall over the decay (bridged-T feel) | 0 to 1 semitone | 0 to 5 semitones |
| Decay | Length | 400 to 2000 ms | 150 to 4000 ms |
| Glide | Portamento between overlapping notes (mono) | 0 to 120 ms | 0 to 400 ms |
| Click level | Front edge | 0 to 0.3 | 0 to 1 |
| Saturation | Drive (how it reads on small speakers) | 0.2 to 0.7 | 0 to 1 (into hard clip) |
| Tone | Low-pass | 1 to 6 kHz | 200 Hz to 16 kHz |

### 8.6 Grit (shared, after any voice)

| Setting | What it does | Core range (proposed) | Wide range (proposed) |
| --- | --- | --- | --- |
| Rate | `degrade~` ratio at 44.1 kHz (effective rate) | 0.44 to 0.91 (19.3 to 40 kHz; SP 0.59, SP pitched down 0.44, MPC60 0.91) | 0.05 to 1 (2.2 to 44.1 kHz) |
| Bits | Quantisation | 10 to 14 (12 is both classics) | 3 to 24 |
| Companding | Linear (SP) to mu-law (MPC60) | 0 or 1 (a switch) | 0 to 1 |
| Filter cutoff, resonance | SSM2044 stand-in after the crusher | 5 to 13 kHz, 0 to 0.3 | 500 Hz to 20 kHz, 0 to 0.9 |
| Filter sweep | Cutoff envelope decay (SP channels 1 and 2) | 0 (off) to 300 ms | 0 to 1000 ms |
| Drive into crusher | Pre-crush saturation | 0.2 to 0.6 | 0 to 1 |
| Hiss | High-passed noise floor, level | -70 to -50 dB | -90 to -30 dB |
| Crackle | Random clicks per second, band 0.8 to 8 kHz | 0 to 12 per second | 0 to 60 per second |
| Pops | Big clicks per second | 0 to 1 per second | 0 to 5 per second |
| Drift | Random pitch per hit (wow stand-in) | 0 to 8 cents | 0 to 50 cents |
| Truncate | Hard gate on the tail | Off, or 150 to 600 ms | 30 to 2000 ms |

## Open questions

1. **Grit per drum or per kit?** Per drum breeds better; per kit makes the album cohere (section 7).
2. **Does the pitch-down trick need a real render-and-replay,** or is a lower `degrade~` rate close enough by ear? Needs a listening spike.
3. **What curve did the MPC60 use?** No source found. Mu-law is a stand-in.
4. **SP-1200 output filter cutoffs** are forum estimates; a measured source (Rossum's reissue documentation, readable once its host is allowed) would firm up the filter ranges.
5. **Should the 808 be its own device** or a Kick Breeder mode (Pitch from note, long decay, glide)? The ranges in 8.5 overlap Kick's Wide ranges a lot, which argues for a mode or a shared voice with different Core ranges.

## Sources

- SP-1200: <https://en.wikipedia.org/wiki/E-mu_SP-1200>, <https://cavac.at/cavacopedia/E-mu%20SP-1200>, <https://www.rossum-electro.com/pages/education-sp-1200>, <https://www.rossum-electro.com/products/sp-1200/>, <https://www.tapatalk.com/groups/sp1200/backlight-t673-s10.html>, <https://www.tapatalk.com/groups/sp1200/tune-decay-t1946.html>, <https://www.tapatalk.com/groups/sp1200/sp-sound-and-sampling-techniques-t827.html>, <https://www.mpc-forums.com/viewtopic.php?p=1549820>, <https://www.mpc-forums.com/viewtopic.php?p=1063109>, <https://gearspace.com/board/electronic-music-instruments-and-electronic-music-production/826280-taxonomy-early-digital-synthesizers.html>, <https://gearspace.com/threads/rzas-drums.1419090/>, <https://synthanatomy.com/2023/05/inphonik-rx1200-sampler-plugin-emulates-the-legendary-12-bit-e-mu-sp-1200.html>
- MPC60: <https://www.muzines.co.uk/articles/akai-mpc60/1079>, <https://www.musicradar.com/news/vintage-music-tech-icons-akai-mpc3000>, <https://cdm.link/?p=164766>, <https://www.mpc-forums.com/viewtopic.php?p=200236>, <https://www.attackmagazine.com/features/interview/roger-linn-swing-groove-magic-mpc-timing/3/>
- SP-303 and Madlib: <https://static.roland.com/manuals/sp-404mk2_reference_v4/en-US/7984244378561675.html>, <https://goodhertz.com/tonal/vinyl-sim>, <https://create.routenote.com/blog/producer-profile-madlib/>
- Lo-fi technique: <https://www.attackmagazine.com/technique/character-crunch-replicating-the-sound-of-vintage-samplers/>, <https://loopcloud.com/cloud/blog/5185-Unlocking-Vintage-Vibes-Mastering-Classic-Sampler-Techniques-in-Modern-Music-Production>
- Vinyl and tape: <https://foliodsp.pages.dev/components/color/vinyl-noise>, <https://sccode.org/1-4Sj>, <https://arxiv.org/pdf/2206.06259v2.pdf>, <https://downloads.izotope.com/docs/ozone8/vintage-tape/index.html>, <https://www.soundonsound.com/techniques/analogue-warmth>, <https://sageaudio.com/articles/tape-emulation>
- Breaks and mixing: <https://fivethirtyeight.com/features/the-most-sampled-song-of-all-time>, <https://producelikeapro.com/blog/5-most-sampled-drum-beats-in-hip-hop/>, <https://www.2pacboard.com/community/threads/2003-dj-premier-interview.9066>, <https://blog.samplefocus.com/blog/dj-premier-breakbeat-hip-hop-samples>, <https://violetrecording.com/how-to-make-boom-bap-beats/>, <https://unison.audio/how-to-eq-drums/>, <https://emastered.com/blog/how-to-eq-drums>
- TR-808: <https://www.manualslib.com/manual/1294765/Roland-Tr-808.html?page=6>, <https://www.kvraudio.com/forum/viewtopic.php?p=7494787>, <https://www.kvraudio.com/forum/viewtopic.php?p=8606994>, <https://speech.di.uoa.gr/ICMC-SMC-2014/images/VOL_2/1453.pdf>, <https://docs.cherryaudio.com/misfit-audio/transistor-808-hats>, <https://www.pressreader.com/australia/computer-music/20190220/282381220809560>, <https://e-licktronic.com/forum/viewtopic.php?p=8499>, <https://en.wikipedia.org/wiki/Tadao_Kikumoto>, <https://www.slate.com/articles/arts/music_box/2008/12/whats_an_808.html>, <https://www.thomannmusic.com/blog/bass-beats-and-legend-the-roland-tr%E2%80%91808-and-what-makes-it-so-iconic/>, <https://orphiq.com/resources/808-drums-explained>
- J Dilla: <https://www.musicradar.com/news/pioneers-j-dilla>, <https://brettworks.com/2013/07/23/roger-linn-on-drum-machine-groove-and-j-dillas-off-beat-sound/>, <https://loudlands1.squarespace.com/blog/drumming-with-microtime-part-1>, <https://www.mpc-forums.com/viewtopic.php?p=412257>
- clipping.: <https://www.perfectcircuit.com/signal/jonathan-snipes-interview>, <https://lamniformes.substack.com/p/drumming-upstream-2-body-and-blood>, <https://www.spinmagazine.com/2014/01/clipping-interview-noise-rap-sub-pop-midcity/>, <https://www.thelineofbestfit.com/features/interviews/nine-songs-clipping>
- Death Grips: <https://talkhouse.com/hunter-hunt-hendrix-of-liturgy-and-survival-talks-death-grips-no-love-deep-web>, <https://www.rollingstone.com/music/music-album-reviews/review-death-grips-bottomless-pit-sounds-like-no-punk-on-earth-122585/>, <https://www.jhunewsletter.com/article/2012/05/death-grips-the-internet-band-of-the-21st-century-19381>
- Max 8: <https://docs.cycling74.com/max8/refpages/degrade~>, <https://docs.cycling74.com/max8/refpages/svf~>, <https://docs.cycling74.com/max8/refpages/mc.overdrive~>, <https://docs.cycling74.com/max8/tutorials/09_dynamicschapter03>
