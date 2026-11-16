# Research: clave, anchors, subdivision and groove data

Researched 2026-11-16, following [groove-and-drum-feel.md](groove-and-drum-feel.md) (2026-11-15). That note covers what makes a groove feel good (syncopation's inverted U, microtiming, swing against tempo, density, 1/f humanise). This one does not repeat it.

The problem: Bloodline's generated drum patterns feel too busy. The user's hypothesis is that a **clave** is missing: a fixed anchor or timeline pattern that outlines the strongest accents, with the groove making small, fairly consistent variations around it. Subdivision (straight sixteenths, swung, triplet or 12/8, mixed and ambiguous feels) needs handling too. The user wants datasets or research that give exact placements (onset times in ms or fractions of a beat, and velocities) to use as training data or hard-coded templates.

The short answer: the hypothesis holds up in real drumming data, and the busyness is mostly not about how many notes there are. Section 7 has the measurements.

## How to read this

- **Method caveat:** this session's network allowed only GitHub raw files (`raw.githubusercontent.com`) and Google Cloud Storage (`storage.googleapis.com`). Every journal, preprint and repository host tried (`arxiv.org`, `doi.org`, `zenodo.org`, `osf.io`, `mtosmt.org`, `magenta.tensorflow.org`, `cgm.cs.mcgill.ca`, `archives.ismir.net`, `duo.uio.no`, `bridgesmathart.org`) was refused by the proxy. So:
  - The **Groove MIDI Dataset was downloaded and analysed directly.** Numbers computed here from that data are marked **[measured]**, and the scripts are listed in section 7.8.
  - Dataset documentation read from loader code and README files on GitHub (Magenta, TensorFlow Datasets, mirdata, GrooveToolbox, MDB Drums, ADTOF) is marked **[read]**, as are this repository's own files.
  - Papers were reached only through search-engine extracts (abstracts, publisher records, citing pages). These are marked **[extract]**. If the extract came from a later paper describing the cited one, it is marked **[extract, second-hand]**.
  - The researcher's reasoning and design suggestions are marked **[inference]**. Anything recalled without a source found in this session is marked **[background, unverified]**.
- **Confidence:** High = several independent sources agree, or a direct measurement on a large sample. Medium = one strong study or one dataset. Low = one study, a disputed result, or theory without direct tests.
- **Grid notation:** a 16-step bar of sixteenths is numbered 0 to 15, so step 0 is beat 1, 4 is beat 2, 8 is beat 3 and 12 is beat 4. The counting is `1 e & a 2 e & a 3 e & a 4 e & a`. Patterns are written `x` (onset) and `.` (rest). A 12-step cycle is one bar of 12/8 in eighths. "Swing" as a single number is the MPC convention from the earlier note: where the second note of a pair lands inside the pair, so 0.50 is straight, 0.667 is a 2:1 triplet feel, and a ratio r:1 equals r / (1 + r).
- **Spelling and dashes:** Australian spelling. No em dashes.

## TL;DR

| # | Finding | Confidence |
|---|---------|------------|
| 1 | **Timelines are short, fixed and nearly maximally even.** The six best-known 4/4 timelines (son, rumba, bossa nova, gahu, shiko and soukous, or "rap" in some lists) all have 5 onsets in 16 pulses. The 12/8 standard bell has 7 onsets in 12 pulses. Tresillo is E(3,8), the bossa nova clave is a rotation of E(5,16), and the standard bell is a rotation of E(7,12) (checked here). Of the 4,368 possible 5-in-16 rhythms, only 16 are almost maximally even, and son clave is the one that also has "rhythmic oddity" (no two onsets half a cycle apart). | High (patterns) / Medium (why they are liked) |
| 2 | **Real grooves are a stable core plus a varied surface, as hypothesised.** In about 400 Groove MIDI Dataset beats, positions a drummer uses in at least 75% of bars ("anchors") are present in 84 to 94% of bars for the kick and snare accents. 40% of bars have exactly the beat's modal kick pattern and 43% the modal snare accents, against only 10% for ghost notes. Most of the bar-to-bar change is ghosts and hats. | Medium-High [measured] |
| 3 | **Variation clusters at phrase ends.** Kick plus snare-accent changes against the beat's modal bar average 2.5 to 2.9 per bar in bars 1 to 3 and 5 to 7 of each eight, then 4.2 in bar 4 and 4.4 in bar 8. Drummers vary or fill on bars 4 and 8, and hold the pattern otherwise. | Medium [measured] |
| 4 | **Bloodline's busyness is mostly incoherence, not note count.** Drawing each sixteenth independently from the GMD's own per-position odds (Bloodline Breaks' method) gives the right density and syncopation. But such bars match the nearest real beat only at Jaccard 0.52 to 0.62, against 0.67 to 0.78 for real bars matched with another real beat (0.63 to 0.81 with their own beat). Per-position odds average many different beats, so independent draws mix several beats' ornaments into one bar. | Medium [measured] |
| 5 | **Idiomatic density** in GMD beats: kick 3.6 to 4.8 per bar, snare 4.2 to 6.9 (2 to 3 of them accents, the rest ghosts), closed hat 5.9 to 6.9 in funk, hip hop and soul, and 14 to 22 onsets a bar in all. Bloodline Breaks' style odds give 12 to 18, which is not too many. | High [measured] |
| 6 | **Swing in GMD's funk, hip hop and soul hats is essentially straight** (median pair split 0.49 to 0.51, upper quartile about 0.54). Jazz ride eighths sit at about 0.66 (1.9:1). New Orleans sixteenths are at about 0.59. Frane's 30 classic breakbeats have a median sixteenth swing of 1.2:1 (about 0.545). Swing is a style setting, small for hip hop, and per part. | Medium |
| 7 | **Per-instrument timing is small and systematic.** Against the hats on the same beat, the kick and the 2-and-4 backbeat land 3 to 8 ms early, while ghost snares on the e and a land 4 to 18 ms late. Jitter within one performance is about 14 to 27 ms SD. The whole kit runs 6 to 17 ms ahead of the click, which fits the known early bias against a metronome but could also be recording latency. | Medium [measured] |
| 8 | **Non-isochronous subdivisions are stable and specifiable.** Malian jembe "Manjanin" splits the beat short-medium-long at about 25:33:42, consistent to the millisecond across performances. No 4/4 drum-kit dataset found here contains this; a template would have to be hard-coded from the paper. | Medium [extract] |
| 9 | **Datasets:** the Groove MIDI Dataset (GMD) and its Expanded version (E-GMD) are the only large, freely licensed sources found with human microtiming and velocity against a known click grid. The Drum Groove Corpora (Hosken et al.) adds timing data from 251 reconstructed commercial grooves and Loop Loft performances as CSV. IEMP gives onset data for Cuban son, Malian jembe and Uruguayan candombe from audio. Drum transcription sets (ENST, IDMT-SMT, MDB, ADTOF, STAR) are audio benchmarks, not groove templates. | High |

## 1. Why the current output may feel busy

### 1.1 What the code does [read]

- **Bloodline Breaks (JUCE)**, [plugins/bloodline-breaks/core/pattern.cpp](../../plugins/bloodline-breaks/core/pattern.cpp). One bar of 16 steps per lane, looped. `PatternOps::fresh` draws **every step of every lane independently**: `rng.chance(odds(l, s))`, where `odds` blends a flat base (`kFlat = {0.2, 0.12, 0.45, 0.1}`) with the style's sixteen-digit odds string by `pull`, then with learned `Taste`. Nothing ties a kick to a snare, a hit to its neighbours, or one lane to another.
  - **Anchors are not guaranteed.** With the default `pull = 0.7`, the Boom bap kick on the one (`9`) has odds 0.2 + (0.9 - 0.2) x 0.7 = 0.69, and each backbeat snare (`9`) has 0.12 + 0.78 x 0.7 = 0.67. So about 31% of fresh bars lack the kick on 1, and only about 44% have both backbeats **[inference, arithmetic]**.
  - **Every "0" step still fires.** A `0` digit keeps `flat x (1 - pull)` odds: 6% for a kick step, 13.5% for a hat. Across the bar that adds 1.0 to 1.7 unplanned hits per bar for most styles **[inference, arithmetic]**.
  - `mutate` reconsiders each step with chance `min(0.9, spread x 0.8)`, again independently, and `cross` swaps whole lanes, so offspring can pair a kick from one parent with a snare from another that was never played with it.
  - Expected hits per bar at `pull = 0.7`: Boom bap 14.8, Lo-fi 12.4, Dilla 13.3, Trap 17.1, Drill 13.1, West coast 15.9 **[inference, arithmetic from the odds strings]**. These are at or below GMD densities (section 4.1), so raw count is not the main problem.
- **Bloodline Breaks 2**, [plugins/bloodline-breaks-2/core/loops.cpp](../../plugins/bloodline-breaks-2/core/loops.cpp). Same style-odds approach (`PatternOps::fresh` adds a hit at each step with `rng.chance(odds(l, s))`), plus free placement off the grid and a `warp()` function for swing.
- **Bloodline Breaks mobile prototype**, [web/bloodline-breaks-mobile-prototype.html](../../web/bloodline-breaks-mobile-prototype.html), the newest. It already does several things this note recommends: `isAnchor` forces the kick on 1 and the snare on 2 and 4, `pickBySync` picks among 14 candidates by a Longuet-Higgins and Lee syncopation target, and later bars "mostly repeat the first". But in `candidate()`, each step of bars 2 onward is redrawn with probability `0.2 + 0.4 x complexity`, the same in every bar. At complexity 0.5 that comes to about 1.5 kick changes per bar **[inference, arithmetic]**, close to GMD's 1.66 (section 3.2), but spread evenly instead of gathered at phrase ends. Bar 1 itself is still an independent draw per step. Fills are only added in the last four steps of the loop.
- **Bloodline Drums 2**, [plugins/bloodline-drums-2/core/kit.cpp](../../plugins/bloodline-drums-2/core/kit.cpp) and [plugin/processor.cpp](../../plugins/bloodline-drums-2/plugin/processor.cpp). This breeds sounds, not patterns. Its `Loop` plays one of three fixed audition patterns (`Quarters`, `Boom bap`, `16ths`), so it is not a source of busyness.

### 1.2 What the data says about it

Section 7.6 tests the Breaks method directly. Take the GMD's own per-position odds for a style, draw kick and snare accents independently per step (as `fresh` does), and compare the bars with real beats. The draws match on density and on kick syncopation, but sit 0.11 to 0.17 Jaccard further from any real beat than real bars do **[measured]**. A per-position probability is an average over many different beats. Each real beat uses a few positions almost always and others rarely, so sampling the average puts a little of every beat's ornament into one bar.

**Takeaway [inference]:** "busy" here mostly means incoherent rather than dense. Many plausible but unrelated hits are spread evenly over the weak positions, with no stable skeleton for them to vary against. A fixed anchor layer, a per-groove "modal bar" that repeats, and a small, structured change budget would address it more than lowering density would.

## 2. Timelines, clave and anchor patterns

### 2.1 The patterns

| Name | Grid | Onsets (steps) | Pattern | Inter-onset intervals | Notes |
|---|---|---|---|---|---|
| Son clave (3-2) | 16 | 0 3 6 10 12 | `x..x..x...x.x...` | 3 3 4 2 4 | IOIs confirmed in a Toussaint extract **[extract]**. 2-3 is the same rotated by 8: `..x.x...x..x..x.` (2 4 8 11 14). |
| Rumba clave (3-2) | 16 | 0 3 7 10 12 | `x..x...x..x.x...` | 3 4 3 2 4 | Third stroke one step later than son **[background, unverified]**. |
| Bossa nova clave | 16 | 0 3 6 10 13 | `x..x..x...x..x..` | 3 3 4 3 3 | A rotation of E(5,16) (checked here, section 2.3). |
| Gahu | 16 | 0 3 6 10 14 | `x..x..x...x...x.` | 3 3 4 4 2 | IOIs confirmed in a course guide extract **[extract]**. |
| Shiko | 16 | 0 4 6 10 12 | `x...x.x...x.x...` | 4 2 4 2 4 | Cinquillo is described as a "double-time" shiko **[extract]**. |
| Soukous | 16 | 0 3 6 10 11 | `x..x..x...xx....` | 3 3 4 1 5 | Listed with the six in some Toussaint sources; others list "rap" instead **[extract, conflicting]**. |
| Tresillo | 8 (x2 in 16) | 0 3 6 / 8 11 14 | `x..x..x.x..x..x.` | 3 3 2 | E(3,8) **[extract]**. The first half of son clave. |
| Cinquillo | 8 | 0 2 3 5 6 | `x.xx.xx.` | 2 1 2 1 2 | E(5,8) **[extract]**. |
| Standard bell (bembé, Ewe "gankogui" family) | 12 | 0 2 4 5 7 9 11 | `x.x.xx.x.x.x` | 2 2 1 2 2 2 1 | 7 strokes in 12 eighths **[extract]**; a rotation of E(7,12) (checked here). Same interval pattern as the diatonic scale (2 2 1 2 2 2 1), Pressing's isomorphism **[extract, second-hand]**. |
| Fume-fume (5 of 12) | 12 | 0 2 4 7 9 | `x.x.x..x.x..` | 2 2 3 2 3 | Exactly E(5,12) as Bresenham generates it (checked here) **[background, unverified as named]**. |
| Backbeat | 16 | 4 12 | `....x.......x...` | 8 8 | Anchor of rock, funk, soul and hip hop. |
| Four on the floor | 16 | 0 4 8 12 | `x...x...x...x...` | 4 4 4 4 | Kick anchor of disco and house; also the dembow kick. |
| Dembow snare (reggaeton) | 16 | 3 6 11 14 | `...x..x....x..x.` | 3 5 3 5 | Over four-on-the-floor kick; each half is tresillo's last two strokes **[background, unverified]**. |
| Amen break, bar 1 | 16 | kick 0 2 10 11; snare 4 7 9 12 15 | K `x.x.......xx....` S `....x..x.x..x..x` | | A common transcription **[background, unverified]**. |

Two readings of the 16-step clave matter for Bloodline. In salsa and son, one clave cycle spans **two 4/4 bars of eighths**, so the 16 steps are eighths. At hip hop or reggaeton tempos the same pattern is often heard as **one bar of sixteenths** (a "cut-time" reading) **[background, unverified]**. Both fit a 16-step lane; the choice sets how fast the timeline feels.

### 2.2 What a timeline is for

- **A fixed reference, not a melody.** Agawu's study of the 12-pulse standard pattern says scholars split on whether the time line is "the structural core" of a piece (Kubik) or a "phrasing referent" that the other parts phrase against. Agawu credits the term "standard pattern" to Desmond Tay, not A. M. Jones **[extract]**. Agawu, K. (2006). Structural analysis or cultural analysis? Competing perspectives on the "standard pattern" of West African rhythm. *Journal of the American Musicological Society* 59(1), 1-46. https://doi.org/10.1525/jams.2006.59.1.1 ; reading copy https://music.arts.uci.edu/abauer/6.2/readings/Agawu_JAMS_06.pdf . **Confidence:** High (that the role is debated) / Medium (either reading).
- **Clave as a matrix.** Peñalosa uses "clave" both for the five-stroke pattern and for the whole system of parts that line up with it or against it ("cometric and contrametric" relations to tresillo and the beat), and derives many Afro-Cuban patterns from the 6/8 bell **[extract, second-hand]**. Peñalosa, D. (2009). *The Clave Matrix: Afro-Cuban Rhythm, Its Principles and African Origins*. Bembe Books. **Confidence:** Medium (book not read).
- **Why these patterns.** Toussaint lists four properties of good timelines: almost maximal evenness, rhythmic oddity, rhythm-shadow contour isomorphism and gestalt despatialisation. Requiring almost maximal evenness cuts the 4,368 possible 5-onset, 16-pulse rhythms to 16, and son clave is the survivor of the oddity test. Rhythmic oddity is Arom's observation that 16-pulse cycles "hardly ever contain notes eight beats apart" **[extract]**. Toussaint, G. T. What makes a "good" rhythm good? (extract via https://www.scribd.com/doc/289224561/What-a-Good-Rhythm-Good ; published form not confirmed). **Confidence:** Medium.
- **Son is the "central" clave.** Among the six 4/4 timelines, son was most similar to all the others under several similarity measures, which Toussaint offers as an explanation for its popularity. Pressing's complexity measure matched how hard the claves are to perform; Lempel-Ziv complexity did not **[extract]**. Toussaint, G. T. (2002). A mathematical analysis of African, Brazilian, and Cuban clave rhythms. *Proc. BRIDGES 2002*, 157-168. https://archive.bridgesmathart.org/2002/bridges2002-157.html . **Confidence:** Medium.
- **Similarity measures.** Hamming, swap and chronotonic distances, compared by how well phylogenetic trees built from them show structure among the binary and ternary timelines **[extract]**. Toussaint, G. T. (2004). A comparison of rhythmic similarity measures. *Proc. ISMIR 2004*. https://archives.ismir.net/ismir2004/paper/000134.pdf . A follow-up found edit distance matched human similarity judgements on six Afro-Cuban timelines better than swap distance **[extract]**. Toussaint, G. T., Campbell, M. & Brown, N. (2011). *Analytical Approaches to World Music* 1(2). https://oldaawmjournal.iftawm.org/articles/2011b/Toussaint_AAWM_Vol_1_2.pdf . **Confidence:** Medium.
- **Off-beatness.** A group-theory property of timelines, proposed as a better predictor of preference than rhythmic oddity and as a possible definition of syncopation **[extract]**. Toussaint, G. T. (2005). Mathematical features for recognizing preference in Sub-Saharan African traditional rhythm timelines. *ICAPR 2005*, LNCS 3686. https://doi.org/10.1007/11551188_2 . The definition was not reached; the usual statement is onsets on pulses not reachable by any regular polygon through pulse 0 of a 12-pulse cycle, that is pulses 1, 5, 7 and 11 **[background, unverified]**. **Confidence:** Low (definition not read).
- **Euclidean rhythms.** Bjorklund's (Euclid's) algorithm spreads k onsets over n pulses as evenly as possible and reproduces many traditional timelines, for example E(3,8) as tresillo and E(5,8) as cinquillo **[extract]**. Toussaint, G. T. (2005). The Euclidean algorithm generates traditional musical rhythms. *Proc. BRIDGES 2005*, 47-56. https://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf . **Confidence:** High.
- **Book.** Toussaint, G. T. (2013; 2nd edn 2020). *The Geometry of Musical Rhythm: What Makes a "Good" Rhythm Good?* Chapman & Hall/CRC **[extract]**. Not read.

### 2.3 Checks run here [measured]

`timelines_and_gmd_clave.py` (section 7.8) confirmed: bossa nova is an E(5,16) necklace (a rotation of `x..x..x..x..x...`), but son, rumba, gahu, shiko and soukous are not. The standard bell is an E(7,12) necklace, and 5-of-12 fume-fume is exactly E(5,12). Son, rumba and bossa have no two onsets 8 steps apart; gahu, shiko and soukous each have one such pair. So son and rumba are "nearly even with oddity", and only bossa is fully Euclidean **[measured]**. **Takeaway [inference]:** `E(k, n)` plus a rotation is a cheap generator for "even" timelines, but the best-known clave is one step off even. A Bloodline clave layer should store named patterns, not just generate Euclidean ones.

### 2.4 Do drummers play the clave in drum-kit grooves? [measured]

In GMD's 55 Latin and Afro-Cuban 4/4 beats, a script looked for a part (cross-stick, snare, rim, kick or ride) whose onsets matched any of the six 16-step timelines in either direction and either reading (two bars of eighths, or one bar of sixteenths), with at least 70% of template strokes present. Only 13 matched. 11 of those were the **kick** on shiko-like positions (`x...x.x...x.x...`) in samba-reggae, ijexá and baião beats; one was rumba-like and one soukous-like. 42 of 55 had no part carrying a clave **[measured]**. **Takeaway [inference]:** in kit drumming the timeline is usually implied by how the kick, snare and hats line up, not played literally on one voice. A Bloodline clave layer can be **heard** (a cross-stick or perc lane) or **silent** (a mask that weights where the other lanes put accents). Both are reasonable; the silent mask is the closer match to kit practice.

### 2.5 Equivalent anchors outside Latin music

- **Backbeat and the one.** Funk is often described as a renewed emphasis on "the One" with a repetitive focus on presence. This was Danielsen describing Scannell's argument **[extract, second-hand]**. Danielsen treats a funk groove as one structural unit heard as a whole, and notes that funk sixteenths can carry swing **[extract, second-hand]**. Danielsen, A. (2006). *Presence and Pleasure: The Funk Grooves of James Brown and Parliament*. Wesleyan University Press. **Confidence:** Medium.
- **Pop patterns as variations of two archetypes.** A corpus study of 2012 to 2021 Billboard pop describes drum patterns as variations of rock or four-on-the-floor beats **[extract]**. Geary, D. (2024). Formal functions of drum patterns in post-millennial pop songs, 2012-2021. *Music Theory Online* 30(2). https://www.mtosmt.org/issues/mto.24.30.2/mto.24.30.2.geary.php . **Confidence:** Medium.
- **In GMD [measured]**, the anchor positions are where theory puts them. The kick on the one is present in 0.78 to 0.91 of bars in hip hop, funk, soul and pop, and 0.99 in afrobeat. The snare on 2 and 4 is present in 0.65 to 0.95 of bars in rock, funk, hip hop, soul and pop. Latin snare has no backbeat (0.35 to 0.57 everywhere) and New Orleans snare is dense everywhere (0.54 to 0.76). Full tables are in section 7.3.

## 3. Fixed core and variable surface

### 3.1 Theory

- **Groove as a kinetic framework.** Pressing describes groove as a framework for reliable prediction, with syncopation working against a steady frame (already in the earlier note) **[extract, second-hand]**. Pressing, J. (2002). Black Atlantic rhythm. *Music Perception* 19(3), 285-310. https://doi.org/10.1525/mp.2002.19.3.285 .
- **Repetition with variation** is the theme of Danielsen 2006 and of the predictive-coding account in the earlier note (Vuust and Witek 2014; Stupacher et al. 2022): prediction is built by repetition and challenged by deviation. **Confidence:** Medium (theory).
- **Fills mark phrase ends.** General references describe fills as varied over a song and as signalling the end of a phrase **[extract]**. No quantitative study of where fills fall was found, so section 3.2 measures it.

### 3.2 Measured in GMD [measured]

Method: each GMD 4/4 beat file of four bars or more is cut into bars on the click grid. For each layer (kick; snare accents, meaning at least 0.65 times the file's 95th-percentile snare velocity; snare ghosts, below that; timekeeper, meaning closed and open hats, or the ride when the ride is used more), each bar's set of sixteenth positions is compared with the file's most common bar. The "skeleton" is kick plus snare accents. The files are long performances, so they include the drummer's own fills and variations. GMD says drummers were asked for "a mix of long sequences (several minutes of continuous playing) and short beats and fills" **[read]**.

All styles pooled (325 to 406 files, depending on layer):

| Layer | Onsets per bar | Bars identical to the beat's modal bar | Mean Jaccard to the modal bar | Mean onsets added or removed per bar | Anchor recall (positions used in 75% of bars, present per bar) | Share of the layer's onsets on anchor positions |
|---|---|---|---|---|---|---|
| Skeleton (kick + snare accents) | 6.4 | 0.31 | 0.65 | 3.10 | | 0.52 |
| Kick | 4.0 | 0.44 | 0.69 | 1.66 | 0.85 to 0.94 by style | 0.54 |
| Snare accents | 2.8 | 0.52 | 0.68 | 1.41 | 0.84 to 0.94 | 0.48 |
| Snare ghosts | 3.7 | 0.21 | 0.42 | 2.86 | (few files have ghost anchors) | 0.13 |
| Timekeeper | 7.8 | 0.40 | 0.75 | 2.24 | 0.83 to 0.94 | 0.45 |

Share of bars by number of onsets changed against the modal bar:

| Layer | 0 | 1 | 2 | 3 | 4 or more |
|---|---|---|---|---|---|
| Kick | 0.40 | 0.18 | 0.14 | 0.10 | 0.18 |
| Snare accents | 0.43 | 0.20 | 0.15 | 0.08 | 0.14 |
| Snare ghosts | 0.10 | 0.28 | 0.18 | 0.13 | 0.31 |
| Timekeeper | 0.27 | 0.14 | 0.11 | 0.08 | 0.40 |

Skeleton changes per bar by position in each eight-bar span, counted from the start of the file (pooled):

| Bar | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| Kick + snare-accent onsets changed | 2.92 | 2.86 | 2.49 | **4.16** | 2.78 | 2.89 | 2.54 | **4.44** |

By style, the skeleton is steadiest in **soul** (Jaccard 0.84, 64% of bars identical, 1.4 changes per bar), then afrobeat, latin, funk and hip hop (0.68 to 0.73), rock (0.63), and loosest in **jazz** (0.42), where comping is meant to vary. Latin kick is very stable (0.82) while its snare accents vary (0.58), which is the timeline-plus-improvisation shape. Full per-style rows are in section 7.5.

**Takeaways [inference]:**

1. The user's hypothesis is supported. Kick and backbeat positions form a core that is present in about 9 of 10 bars. Ghost notes are the most variable layer (only 10% of bars repeat the modal ghost pattern exactly) and carry little of the identity.
2. "Small variation" in numbers: in a typical bar a drummer changes about 1.5 kick and 1.4 snare-accent onsets against the modal bar, but this average hides a skew. 40% of bars change nothing in the kick, and the changes pile up in bars 4 and 8 (about 1.5 times the other bars).
3. Changes are mostly additions off the anchors, not removals of anchors. Anchor recall is near 0.9 while 0.4 to 1.8 extra onsets per bar land elsewhere (section 7.5).

## 4. Density and metrical weight

### 4.1 Idiomatic onsets per bar [measured]

| Style | Files | Median BPM | Kick | Snare (all) | Closed hat | Open hat | Pedal hat | Ride | Toms | Total |
|---|---|---|---|---|---|---|---|---|---|---|
| Rock | 144 | 102 | 3.6 | 4.2 | 2.7 | 0.8 | 2.0 | 2.5 | 1.6 | 17.9 |
| Funk | 50 | 100 | 4.1 | 6.9 | 5.9 | 1.0 | 2.1 | 1.4 | 0.4 | 21.9 |
| Hip hop | 30 | 90 | 4.6 | 4.3 | 6.9 | 1.4 | 1.1 | 0.6 | 0.4 | 19.5 |
| Soul | 28 | 102 | 3.8 | 4.6 | 6.1 | 0.6 | 1.3 | 1.3 | 0.3 | 18.1 |
| Latin | 48 | 107 | 5.4 | 9.0 | 1.7 | 0.2 | 3.7 | 3.1 | 2.4 | 25.7 |
| Jazz | 42 | 122 | 2.3 | 4.6 | 0.5 | 0.3 | 2.2 | 3.3 | 1.6 | 14.9 |
| Pop | 15 | 138 | 2.8 | 2.8 | 4.6 | 0.3 | 1.1 | 1.6 | 0.6 | 13.8 |

The totals include crashes (0.1 to 0.5 per bar, not shown). Snare counts include ghosts, flams and the TD-11's rim and cross-stick notes, so "snare 6.9" in funk is about 2.8 accents plus 3.9 ghosts. Bloodline Breaks' Boom bap draws about 3.4 kick, 2.3 snare, 8.3 hat and 0.8 perc per bar (section 1.1): fewer than GMD hip hop, not more.

### 4.2 Metrical weight and generative density control

- **kin.rhythmicator.** A stochastic generator that gives each pulse a weight by Barlow's indispensability (computed from the prime factors of each metrical level). Mapping weights straight to trigger probabilities gives patterns typical of a metre; controls set density per bar, amount of variation, syncopation, metrical strength and the metre itself. A companion, kin.recombinator, recombines MIDI drum loops in real time "to avoid excessive repetition" **[extract]**. Sioros, G. & Guedes, C. (2011). A formal approach for high-level automatic rhythm generation. *Proc. BRIDGES 2011*. https://static1.bridgesmathart.org/2011/cdrom/proceedings/49/paper_49.pdf ; and (2011) Automatic rhythmic performance in Max/MSP: the kin.rhythmicator, *Proc. NIME*, https://repositorio.inesctec.pt/handle/123456789/2132 . **Confidence:** Medium. **Note [inference]:** this is close to what Bloodline does now (weights to independent probabilities). The authors pitch its output as "generic" for a metre rather than a style, which fits the coherence problem in section 1.
- **Syncopation as transformation.** Sioros and colleagues syncopate or de-syncopate a pattern by a small set of displacements against a metrical template built on Longuet-Higgins and Lee weights (the Syncopalooza tool, for MIDI; later for audio loops) **[extract]**. Sioros, G., Miron, M., Cocharro, D., Guedes, C. & Gouyon, F. (2013). Syncopalooza: manipulating the syncopation in rhythmic performances. *Proc. CMMR 2013*, 454-469; Sioros, G. (2015), *Syncopation as Transformation*, PhD thesis, University of Porto, https://repositorio-aberto.up.pt/handle/10216/96185 . **Confidence:** Medium. **Note [inference]:** displacement keeps the number of notes fixed and moves them by metrical level, which suits a variation layer that must not add density.
- **Descriptors and similarity spaces.** Symbolic drum descriptors (frequency bands, syncopation, density, number of instruments, complexity) were tested against listener similarity; syncopation and density predicted where patterns fall in perceptual space, and the maps support interpolation between patterns **[extract]**. Gómez-Marín, D., Jordà, S. & Herrera, P. (2020). Drum rhythm spaces: from polyphonic similarity to generative maps. *Journal of New Music Research* 49(5). https://doi.org/10.1080/09298215.2020.1806887 ; code https://github.com/danielgomezmarin/rhythmtoolbox . **Confidence:** Medium.
- **GrooveToolbox** (Bruford) computes density, syncopation (per band and polyphonic), weak-to-strong ratio and microtiming features from MIDI loops **[read]**: https://github.com/fredbru/GrooveToolbox . Two things in its code are useful here. Its monophonic syncopation uses the profile `5 1 2 1 3 1 2 1 4 1 2 1 3 1 2 1` per bar. Its polyphonic syncopation implements Witek et al.'s cross-instrument measure: a kick before a gap scores the metrical difference **+2** if the next hit is snare (or snare and hat), **+5** if only hat, **+6** if a rest; a snare scores **+1**, **+5** and **+6** likewise. Its "swingness" counts sixteenth positions 3, 7, 11 and 15 more than 25 ms early, and `get_swing_ratio` is unimplemented **[read]**. This gives the per-instrument weights the earlier note could not reach, though as implemented, not as published. **Confidence:** Medium.
- **GrooVAE.** Gillick et al. train models to invert transformations: Humanise (quantised to performed), Drum Infilling (add a missing voice) and Tap2Drum (rhythm to full kit) **[extract]**. Gillick, J., Roberts, A., Engel, J., Eck, D. & Bamman, D. (2019). Learning to groove with inverse sequence transformations. *ICML 2019*. https://arxiv.org/abs/1905.06118 . The representation is, per sixteenth step and per each of 9 drum classes, a triple (hit 0 or 1, velocity 0 to 1, offset -0.5 to 0.5 of a step from the nearest step), so one bar is a 16 x 27 matrix **[read]** (`GrooveConverter` in https://github.com/magenta/magenta/blob/main/magenta/models/music_vae/data.py). Released configs include `groovae_2bar_humanize`, `groovae_2bar_tap_fixed_velocity`, `..._note_dropout` (input notes dropped with up to 0.8 probability, quieter notes first), `groovae_2bar_add_closed_hh` and `groovae_2bar_hits_control` (hits passed as controls so the model only learns velocity and offset) **[read]** (`configs.py`, same repo). The note-dropout and hits-as-controls configs are the nearest to a density knob. Magenta Studio's Drumify and Groove plugins are built on these models **[extract]**. **Confidence:** High (representation) / Medium (results not read).

**Takeaway [inference]:** metrical weight should set where **anchors and accents** go. Density is better set by how many ornament onsets a variation may add on top of a fixed body, than by scaling all per-step probabilities.

## 5. Subdivision and feel

### 5.1 Swing

- **GMD [measured].** The pairwise measure takes, for each eighth (or quarter), the hat or ride onset nearest its start, one in the middle region and the next start, and returns (middle - start) / (next start - start). The median per file, then the median across files:

| Style | Unit (timekeeper) | Files | Median | Quartiles | Ratio |
|---|---|---|---|---|---|
| Funk | 16th pairs (hat) | 25 | 0.502 | 0.496 to 0.545 | 1.01:1 |
| Hip hop | 16th pairs (hat) | 18 | 0.491 | 0.484 to 0.520 | 0.96:1 |
| Soul | 16th pairs (hat) | 7 | 0.500 | 0.494 to 0.589 | 1.00:1 |
| Rock | 8th pairs (hat) | 61 | 0.507 | 0.498 to 0.642 | 1.03:1 (upper quartile: shuffles) |
| Latin | 8th pairs (ride) | 19 | 0.495 | 0.486 to 0.499 | 0.98:1 |
| New Orleans | 16th pairs (hat) | 3 | 0.587 | 0.574 to 0.635 | 1.42:1 |
| Jazz | 8th pairs (ride) | 18 | 0.660 | 0.520 to 0.674 | 1.94:1 |
| Jazz | 8th pairs (hat) | 5 | 0.685 | 0.645 to 0.711 | 2.17:1 |

  Jazz is bimodal: the straight files are labelled fusion, funk and linear. Swing-labelled jazz ride runs 1.75 to 2.06:1 at 124 to 215 BPM and 1.19:1 at 290 BPM, so 20 files show no clear tempo trend (r = -0.03), apart from the very fast one **[measured]**. **Confidence:** Medium (10 drummers).
- **Classic breakbeats swing a little.** In 30 breaks from hip hop's breakbeat canon, sixteenth swing was common but mild: median 1.2:1, about 0.545 or 54.5% **[extract]**. Frane also defines "swing density", the share of swing-level even divisions that contain events, and relates it to how strongly swing is felt **[extract]**. Frane, A. V. (2017). Swing rhythm in classic drum breaks from hip-hop's breakbeat canon. *Music Perception* 34(3), 291-302. https://doi.org/10.1525/mp.2017.34.3.291 . **Confidence:** Medium. **Note [inference]:** GMD's hip hop drummers played straighter than the sampled breaks; Bloodline's Boom bap default of 0.58 is above both. A default of about 0.54 for breakbeat styles and 0.50 for modern hip hop would follow the data.
- **Jazz ride swing falls with tempo** (Friberg and Sundström 2002; Honing and de Haas 2008; in the earlier note). Ride-cymbal swing has been estimated automatically from Weimar Jazz Database excerpts; an autocorrelation method was more reliable than onset detection **[extract]**. Dittmar, C., Pfleiderer, M. & Müller, M. (2015). Automated estimation of ride cymbal swing ratios in jazz recordings. *Proc. ISMIR 2015*. https://www.ismir2015.uma.es/articles/143_Paper.pdf . Follow-up: Dittmar, C., Pfleiderer, M., Balke, S. & Müller, M. (2018). A swingogram representation for tracking micro-rhythmic variation in jazz performances. *Journal of New Music Research* 47(2). https://doi.org/10.1080/09298215.2017.1367405 ; examples https://www.audiolabs-erlangen.com/resources/MIR/2017-JNMR-SwingRatio . Workshop slides mention a preferred ratio near 2.0 and no strong correlation **[extract, unverified]**. **Confidence:** Medium.
- **Soloists against drummers.** Friberg and Sundström found soloists swinging less than the drummer (earlier note). Corcoran and Frieler analysed soloists' swing eighths in the Weimar Jazz Database; the findings were not reached **[extract]**. Corcoran, C. & Frieler, K. (2021). Playing it straight: analyzing jazz soloists' swing eighth-note distributions with the Weimar Jazz Database. *Music Perception* 38(4), 372-385. https://doi.org/10.1525/mp.2021.38.4.372 . Benadon's beat-upbeat ratios span 1.0 to 3.0 (earlier note). **Confidence:** Low here.

### 5.2 Non-isochronous subdivision

- **Jembe (Mali).** Polak measured millisecond inter-onset intervals in multitrack recordings of jembe ensembles and found beat subdivisions that are consistently unequal, not swung triplets or straight sixteenths. In "Manjanin" the ternary subdivision runs short-medium-long at about **25:33:42**. The 2010 paper also describes two further ternary patterns and one quaternary pattern **[extract]**. Polak, R. (2010). Rhythmic feel as meter: non-isochronous beat subdivision in jembe music from Mali. *Music Theory Online* 16(4). https://mtosmt.org/issues/mto.10.16.4/mto.10.16.4.polak.html ; ratio from Polak's project page, https://www.rainerpolak.de/research-projects/swing-based-meter/empirical-consistency/ . **Confidence:** Medium (ratio from an author summary, not the tables).
- **Across pieces and genres.** A corpus of 15 performances of "Manjanin", "Maraka" and "Woloso" (about 43,000 data points) found subdivision ratios gravitating to specific values with small SDs **[extract]**. Polak, R., London, J. & Jacoby, N. (2016). Both isochronous and non-isochronous metrical subdivision afford precise and stable ensemble entrainment. *Frontiers in Neuroscience* 10, 285. https://doi.org/10.3389/fnins.2016.00285 . The same consistency appears in Bamana bòn and Khasonka dundunba drumming ("Ngòn Fariman", "Bire"); the authors compare subdivision timing with jazz swing ratios **[extract]**. Polak, R. & London, J. (2014). Timing and meter in Mande drumming from Mali. *Music Theory Online* 20(1). https://doi.org/10.30535/mto.20.1.1 . Long, medium and short (LMS, LSS) labels are used for these feels in later work **[extract]**. **Confidence:** Medium.
- **Candombe.** The chico drum is the timekeeper, repeating a one-beat pattern; the piano drum carries the timeline with one-cycle patterns; the repique improvises **[extract]**. Microtiming profiles at the sixteenth level have been tracked jointly with beats in candombe and samba timekeepers, and in maracatu they vary over time within pieces **[extract]**. Fuentes, M., Maia, L. S., Rocamora, M., Biscainho, L. W. P., Crayencour, H. C., Essid, S. & Bello, J. P. (2019). Tracking beats and microtiming in Afro-Latin American music using conditional random fields and deep learning. *Proc. ISMIR 2019* (PDF via https://core.ac.uk/download/487523371.pdf). The profile values were not reached. **Confidence:** Low (values).
- **Wonky, quintuplet and "drunk" feels.** No primary measurement of 5:4 (quintuplet) hip hop feel or of J Dilla's timing was found in this session. The previous note's sources and Frane 2017 are the nearest. **Confidence:** none (absence of evidence).

### 5.3 Per-instrument timing [measured]

Mean offset of each instrument, relative to the same file's closed hat (or ride) on the beat, averaged over files. Positive is later. The mean of per-file means:

| Style | Kick on beats | Kick on & | Snare on beats (incl. 2 and 4) | Snare on & | Snare on e/a (mostly ghosts) | Hat on & | Hat on e/a |
|---|---|---|---|---|---|---|---|
| Rock | -4.6 | +2.3 | -2.8 | 0.0 | +4.1 | +5.7 | +1.0 |
| Funk | -8.1 | -11.0 | -5.2 | -9.9 | +9.0 | -1.1 | +3.0 |
| Hip hop | -5.3 | -5.2 | -3.5 | +2.3 | +14.3 | -1.7 | +3.3 |
| Soul | -5.3 | -8.0 | -3.5 | -18.5 | +17.6 | -3.8 | +12.7 |
| Pop | -3.1 | -8.3 | -3.7 | -2.1 | +12.8 | -8.2 | . |
| Afrobeat | -6.8 | -9.1 | -5.3 | -13.9 | +7.0 | -1.8 | +1.8 |

Absolute offsets against the click are larger and mostly early: closed hat on the beat -6 to -17 ms, kick on the beat -11 to -19 ms in funk, hip hop, soul and pop (section 7.4). Jitter within one file is about 14 to 27 ms SD on the beat; the systematic per-file offset on the beat varies by about 3 to 14 ms SD between files.

**Reading [inference]:** in GMD the kick and the backbeat sit a few milliseconds **ahead** of the hats, and ghost notes sit 4 to 18 ms **behind**, so a "late snare" is a ghost-note trait here, not a backbeat trait. That disagrees with the "laid-back backbeat" default in the mobile prototype's `defaultFeel` (snare `behind` 60% of the time). Two caveats. First, the early bias against the click fits Câmara et al.'s finding that playing to a metronome gave earlier mean onsets than playing to a backing track, possibly "negative mean asynchrony" **[extract]** (Câmara, G. S., Nymoen, K., Lartillot, O. & Danielsen, A. (2020), *Music Perception* 38(1), 1-26, https://doi.org/10.1525/mp.2020.38.1.1); TD-11 or recording latency could also contribute. Second, these drummers were not asked for any feel. When ten drummers were asked to play laid back at 96 BPM, they delayed snare attacks by **17.4 ms** on average **[extract, second-hand]** (Danielsen et al. 2015, *JASA* 138(4), 2301-2316, https://doi.org/10.1121/1.4930950). Relative offsets between instruments are the trustworthy part. A GMD-like default is kick -5, backbeat -3, hats 0, ghosts +10 ms, with "laid back" an extra 10 to 20 ms on the snare by choice.

### 5.4 Representing subdivision per instrument

**[inference]** The evidence points to subdivision being a property of each **lane and style**, not of the whole kit: GMD jazz swings the ride while the kick is not swung; funk hats are straight while ghosts drag; jembe subdivisions are stable non-even ratios. A compact representation is a **beat map** per lane: the fractions of a beat where that lane's steps fall.

| Feel | Steps per beat | Beat map |
|---|---|---|
| Straight sixteenths | 4 | 0, 0.25, 0.5, 0.75 |
| Breakbeat swing 54.5% (Frane median) | 4 | 0, 0.2725, 0.5, 0.7725 |
| MPC 58% | 4 | 0, 0.29, 0.5, 0.79 |
| Triplet eighths (12/8, shuffle) | 3 | 0, 0.333, 0.667 |
| Jazz ride, about 2:1 | 2 | 0, 0.66 |
| Jembe Manjanin, short-medium-long 25:33:42 | 3 | 0, 0.25, 0.58 |
| Quintuplet ("wonky") | 5 | 0, 0.2, 0.4, 0.6, 0.8 |

Bloodline Breaks 2 already has `warp(pos, swing)`, which stretches the first sixteenth of each eighth; a beat map generalises it to any number of steps per beat and to unequal splits.

## 6. Datasets with exact placements

| Dataset | Contents | Size | Genres | Capture | Timing | Velocity | Grid reference | Licence | Format, access |
|---|---|---|---|---|---|---|---|---|---|
| **Groove MIDI Dataset (GMD) v1.0.0** | Human kit performances, MIDI plus synthesised audio | 1,150 files, 13.6 h, over 22,000 bars, 10 drummers (over 80% of duration from hired professionals) **[read]** | 18 labelled styles; rock, funk, latin, jazz, hip hop, soul largest (section 7.1) | Roland TD-11 electronic kit, played to a click at a set tempo; audio aligned to MIDI within 2 ms **[read]** | Raw MIDI (480 ticks per beat, so about 1 ms at 120 BPM) **[measured]** | Yes, 1 to 127 | Yes: MIDI tempo equals the click, beat 1 at time 0 (first notes a few ticks after 0) **[measured]** | CC BY 4.0 **[read]** | `.mid` plus `info.csv` (drummer, session, style, bpm, beat or fill, time signature, split). MIDI-only zip 3.3 MB: https://storage.googleapis.com/magentadata/datasets/groove/groove-v1.0.0-midionly.zip ; with audio 5.1 GB |
| **Expanded GMD (E-GMD) v1.0.0** | The GMD performances re-rendered through many TD-17 kits, for transcription | 45,537 rows, 43 kits, 444.5 h **[read, from the CSV]** | As GMD | Same performances as GMD, so the same timing | MIDI | Yes | Yes | CC BY 4.0 **[background, unverified]** | MIDI zip 107 MB: https://storage.googleapis.com/magentadata/datasets/e-gmd/v1.0.0/e-gmd-v1.0.0-midi.zip ; CSV https://storage.googleapis.com/magentadata/datasets/e-gmd/v1.0.0/e-gmd-v1.0.0.csv ; full 96 GB. Callender, L., Hawthorne, C. & Engel, J. (2020), arXiv 2004.00188 |
| **Drum Groove Corpora** (Hosken et al. 2021) | Onset timing of kit grooves: new Loop Loft corpus, the **Lucerne Groove Research Library** (251 grooves reconstructed from commercial recordings by 50 drummers), and a filtered GMD **[extract]** | 287,328 onset rows **[extract]** | Pop, rock, funk, soul, disco, R&B, hip hop, drum and bass, techno, metal | Loop Loft and GMD played to click (click times not available); Lucerne from recordings | Per onset | Not confirmed | Bars and positions annotated | CC 4.0 **[extract]** | `DrumGrooveCorpora.csv` at https://osf.io/3sejt/ ; paper https://doi.org/10.18061/emr.v16i1.7642 |
| **Senn et al. stimulus sets** | 248 reconstructed drum patterns (2018) and 40 idiomatic pop patterns with complexity ratings (2023) | 248; 40 | Pop, rock, funk, soul, disco | Re-recorded or programmed from transcriptions | Not reached | Not reached | Yes | Not reached | Via the papers in the earlier note (https://doi.org/10.1371/journal.pone.0199604 ; https://doi.org/10.1177/20592043231202576 ). The Lucerne library above is the related corpus. |
| **IEMP Data Collection** (Clayton et al. 2021) | Media plus computationally extracted onset timing, with manual metrical annotation, for six corpora incl. **Cuban son and salsa, Malian jembe, Uruguayan candombe** **[extract]** | Not reached | As named | Audio and video of live ensembles; onsets extracted from audio | Per onset (ms) | Not stated | Onsets assigned to metrical positions (spreadsheet and Matlab tools shared) **[extract]** | Not reached | OSF: https://osf.io/37fws/wiki , code https://osf.io/nvr73 ; paper https://doi.org/10.18061/emr.v16i1.7555 |
| **Candombe dataset** (Nunes, Rocamora, Jure, Biscainho 2015) | 35 performances, 3 to 5 drums, 26 players, over 2 h **[read]** | Over 4,700 annotated downbeats | Uruguayan candombe | Studio audio | **Beats and downbeats only, not onsets** | No | Beat grid | CC BY 4.0 **[read]** | FLAC plus CSV: https://zenodo.org/record/6533068 |
| **Weimar Jazz Database** (Jazzomat) | 456 transcribed jazz solos **[extract]** with beat tracks | 456 | Jazz | Manual transcription from recordings | Per note | Loudness estimates **[background, unverified]** | Beat track | Not reached | https://jazzomat.hfm-weimar.de ; drum swing has to be estimated from audio (Dittmar et al. 2015) |
| **Lakh MIDI Dataset** | 176,581 deduplicated MIDI files, 45,129 matched to the Million Song Dataset **[extract]** | | All | Mostly sequenced, often quantised **[background, unverified]** | MIDI | Usually programmed | Usually yes | CC BY 4.0 **[extract]** | https://colinraffel.com/projects/lmd/ . Good for anchor statistics by genre, not for feel. |
| **Slakh2100** | Lakh-derived multitrack renders for source separation **[extract]** | 2,100 tracks **[background, unverified]** | Mixed | Synthesised from Lakh MIDI | Inherits Lakh MIDI | Inherits | Yes | CC BY 4.0 **[background, unverified]** | Same caveat as Lakh |
| **ENST-Drums** | Three professional drummers, 8 audio and 2 video channels, about 75 min per drummer **[extract]** | | Various | Acoustic kit | Onset annotations (20 labels) | No | No | Custom licence, accept to download **[extract]** | https://adasp.telecom-paris.fr/resources/2009-11-25-enst-drums |
| **IDMT-SMT-Drums** | 608 WAV files, about 2 h 10 min **[extract]** | | Short kick, snare, hat loops | Real, sampled and synthetic | Onsets | No | No | CC BY-SA 4.0 per a secondary source **[extract, second-hand]** | https://zenodo.org/records/7544164 |
| **MDB Drums** | 23 MedleyDB tracks, 7,994 onsets in 6 classes and 21 subclasses **[read]** | | Various | Real recordings | Onsets | No | No | CC BY-NC-SA 4.0 **[read]** | https://github.com/CarlSouthall/MDBDrums |
| **ADTOF** | 359 h from rhythm-game charts, distributed as mel spectrograms **[read]** | | Rock, metal, pop | Crowdsourced charts aligned to audio | Chart grid | No | Chart | CC BY-NC-SA 4.0 (repo) **[read]** | https://github.com/MZehren/ADTOF ; data on request via Zenodo |
| **STAR Drums** | Real non-drum stems plus synthesised drums from automatically transcribed drum stems **[extract]** | Not reached | Various | Source separation plus transcription | Synthetic | Synthetic | Yes | Paper CC BY 4.0; data licence not confirmed **[extract]** | https://doi.org/10.5334/tismir.244 |

**Which have human microtiming and velocity against a known grid:** GMD and E-GMD (MIDI, click-aligned, velocity), and the Drum Groove Corpora (timing; velocity not confirmed). IEMP has human timing from audio with metrical annotation but no MIDI velocity. Candombe has only beats. Lakh and Slakh are mostly quantised programming. ENST, IDMT, MDB, ADTOF and STAR are transcription benchmarks. **No clave MIDI library with measured timing was found**; Toussaint's catalogue is onsets only.

**GMD pitch mapping (Roland TD-11), with Magenta's 9-class grouping** **[read]** (mirdata `groove_midi.py`; Magenta `ROLAND_DRUM_PITCH_CLASSES`):

| Class | Notes |
|---|---|
| Kick | 36 |
| Snare | 38 (head), 40 (rim), 37 (cross-stick) |
| Closed hi-hat | 42 (bow), 22 (edge), 44 (pedal) |
| Open hi-hat | 46 (bow), 26 (edge) |
| Low tom | 43 (head), 58 (rim) |
| Mid tom | 47 (rim), 45 (head) |
| High tom | 50 (rim), 48 (head) |
| Crash | 49 (bow), 55 (edge), 57 (crash 2 bow), 52 (crash 2 edge) |
| Ride | 51 (bow), 59 (edge), 53 (bell) |

Note the non-General-MIDI numbers (22, 26 and 58), and that Magenta folds the pedal hat (44) into closed hat. This note's scripts keep the pedal hat separate (`hhp`), because it is a timekeeper in jazz and Latin.

**Extracting a groove template [inference]:** per beat file, quantise each onset to the nearest sixteenth (or nearest triplet step for swung styles); per lane and position, keep presence probability across bars, mean velocity, mean offset in fractions of a step, and SD. Then take the modal bar and its anchors (presence of 0.75 or more). That gives exactly the "odds string, velocity ladder, offset per step" shape that `StyleInfo` uses, but **per beat instead of per style**, which is what the coherence result in section 1.2 calls for. The scripts in section 7.8 do this; adding a JSON export per file is a few lines.

## 7. Measured numbers from GMD

All numbers in this section are **[measured]** on GMD v1.0.0 "beat" files in 4/4 with at least four bars (412 files, about 20,300 bars), quantised to the click grid. Fill files are excluded.

### 7.1 Coverage

| Style | Files | Bars | Median BPM | BPM range |
|---|---|---|---|---|
| Rock | 144 | 6,284 | 102 | 50 to 200 |
| Funk | 50 | 2,429 | 100 | 80 to 138 |
| Hip hop | 30 | 837 | 90 | 67 to 140 |
| Soul | 28 | 598 | 102 | 80 to 148 |
| Latin | 48 | 3,547 | 107 | 80 to 184 |
| Jazz | 42 | 2,228 | 122 | 88 to 290 |
| Afrobeat | 13 | 979 | 98 | 85 to 114 |
| Pop | 15 | 306 | 138 | 83 to 142 |
| New Orleans | 13 | 723 | 93 | 82 to 124 |
| Afro-Cuban | 7 | 771 | 110 | 105 to 128 |

### 7.2 Velocity ladders (MIDI 1 to 127)

| Style | Kick on 1 | Other kicks | Snare on 2 and 4 | Snare ghosts | Hat on beat | Hat on & | Hat on e/a | Ghost / backbeat | Hat e,a / beat |
|---|---|---|---|---|---|---|---|---|---|
| Rock | 80 | 69 | 114 | 37 | 95 | 74 | 61 | 0.32 | 0.65 |
| Funk | 73 | 57 | 115 | 34 | 83 | 63 | 38 | 0.29 | 0.46 |
| Hip hop | 81 | 59 | 121 | 34 | 98 | 65 | 48 | 0.28 | 0.49 |
| Soul | 52 | 43 | 109 | 29 | 82 | 69 | 50 | 0.26 | 0.61 |
| Latin | 49 | 38 | 97 | 38 | 66 | 59 | 59 | 0.39 | 0.89 |
| Jazz | 76 | 67 | 105 | 39 | 71 | 92 | 52 | 0.37 | 0.73 |
| Pop | 56 | 50 | 112 | 34 | 105 | 38 | 55 | 0.31 | 0.53 |

Snare velocity across all beats is bimodal: a ghost peak at 30 to 49 and an accent peak at 120 to 127, with a trough at 80 to 89. Bloodline Breaks' `velocityFor` already sits close to this: snare ghosts at 0.35 of the accent (GMD 0.26 to 0.32), and hats at 0.9, 0.68 and 0.46 for beat, & and e/a (GMD hip hop 1 : 0.66 : 0.49). **Velocity is not the cause of busyness.**

### 7.3 Onset probability by sixteenth, pooled per style

Probability that a bar has the instrument at that step, written as tenths in the same digit-string format as `StyleInfo::odds` (9 means 0.85 or more):

| Style | Kick | Snare (all) | Closed hat |
|---|---|---|---|
| Rock | `7022302241313121` | `1221712222327222` | `2121212122212121` (ride often used) |
| Funk | `8034121331521211` | `2421842525227335` | `5253626362425242` |
| Hip hop | `8043124342412223` | `1111811222126221` | `6362736273626352` |
| Soul | `9111202142602123` | `2210912324118222` | `6161517352615163` |
| Latin | `8104504262045142` | `4456455545553445` | ride `4131413141303120` |
| Jazz | `5011201130212011` | `2222422323324233` | ride `4011502240114022` |

These pooled strings are what a style-odds generator would use. Section 7.6 shows why sampling them independently is the problem: per beat, the same positions split into near-certain anchors and rare ornaments.

### 7.4 Onset deviation from the nearest sixteenth (ms, positive late), by position class

| Style | Inst | Beat: mean (SD) | &: mean (SD) | e/a: mean (SD) |
|---|---|---|---|---|
| Funk (100 BPM, 16th = 150 ms) | Kick | -11.7 (23.6) | -15.0 (28.2) | +3.6 (32.1) |
| | Snare | -13.1 (27.5) | -18.3 (31.8) | +6.2 (34.3) |
| | Closed hat | -6.3 (18.2) | -8.4 (19.4) | -7.7 (24.4) |
| Hip hop (90 BPM, 16th = 167 ms) | Kick | -16.2 (21.7) | -17.3 (21.7) | +0.5 (38.1) |
| | Snare | -12.0 (29.2) | -11.3 (45.8) | +8.6 (41.1) |
| | Closed hat | -11.5 (21.1) | -12.3 (22.8) | -13.4 (29.5) |
| Soul (102 BPM) | Kick | -19.1 (23.5) | -17.9 (24.5) | +2.3 (30.7) |
| | Snare | -13.7 (22.2) | -28.3 (28.9) | +1.9 (34.4) |
| | Closed hat | -9.6 (18.4) | -13.0 (19.1) | -3.9 (20.7) |
| Rock (102 BPM) | Kick | -5.0 (26.6) | -4.5 (29.5) | -3.4 (38.0) |
| | Snare | -4.0 (27.7) | +2.7 (34.2) | +0.1 (34.5) |
| | Ride | +4.0 (26.9) | +4.6 (28.9) | +12.2 (43.6) |
| Latin (107 BPM) | Kick | +0.2 (27.4) | -0.2 (33.4) | -23.2 (29.0) |
| | Ride | -7.8 (16.4) | -11.5 (19.7) | -8.2 (30.3) |

In fractions of a sixteenth, beat-position SDs are 0.12 to 0.21, and means sit between -0.19 and +0.03 except for swung positions. The SDs pool across drummers and files. Within one file, jitter is about 14 to 27 ms SD on the beat, and systematic per-file offsets on the beat vary by about 3 to 14 ms SD between files ("Systematic vs jitter" table in the script output). For jazz, the sixteenth grid misplaces triplet upbeats, so use section 5.1 instead.

### 7.5 Consistency per style

| Style | Skeleton Jaccard to the modal bar | Kick | Snare accents | Snare ghosts | Timekeeper | Kick anchors (count, recall, extras per bar) | Snare-accent anchors |
|---|---|---|---|---|---|---|---|
| Rock | 0.63 | 0.65 | 0.71 | 0.36 | 0.71 | 2.5, 0.89, 1.62 | 1.9, 0.92, 0.98 |
| Funk | 0.69 | 0.73 | 0.74 | 0.51 | 0.83 | 2.8, 0.91, 1.42 | 1.8, 0.92, 1.24 |
| Hip hop | 0.68 | 0.69 | 0.75 | 0.45 | 0.80 | 3.3, 0.93, 1.83 | 1.5, 0.94, 0.74 |
| Soul | 0.84 | 0.84 | 0.90 | 0.44 | 0.85 | 3.0, 0.94, 0.76 | 2.0, 0.94, 0.43 |
| Latin | 0.71 | 0.82 | 0.58 | 0.49 | 0.76 | 4.2, 0.93, 0.94 | 2.7, 0.88, 1.49 |
| Jazz | 0.42 | 0.51 | 0.44 | 0.40 | 0.64 | 2.0, 0.85, 1.54 | 1.6, 0.84, 1.42 |
| Afrobeat | 0.73 | 0.78 | 0.74 | 0.41 | 0.79 | 4.0, 0.93, 1.38 | 2.6, 0.93, 1.54 |
| Pop | 0.67 | 0.64 | 0.84 | 0.37 | 0.72 | 1.5, 0.88, 1.33 | 1.7, 0.90, 0.40 |

Using two-bar units instead of one-bar units lowers Jaccard slightly (pooled skeleton 0.61 against 0.65), so most of these beats are one-bar patterns with variation, not two-bar patterns.

### 7.6 Independent per-step draws against real beats

For each style, kick and snare accents were drawn per step from the style's pooled per-position probabilities (3,000 bars). Each drawn bar was compared with the modal bars of all real beats in the style; each real bar with its own modal bar and with the best other beat's modal bar.

| Style | Files | Real bar vs own modal bar | Real bar vs best other beat | Independent draw vs best real beat | Kick syncopation (LHL style): real modal bars / draws |
|---|---|---|---|---|---|
| Hip hop | 30 | 0.68 | 0.69 | **0.52** | 3.4 / 3.0 |
| Funk | 50 | 0.65 | 0.70 | **0.55** | 3.2 / 3.0 |
| Rock | 144 | 0.63 | 0.67 | **0.56** | 1.8 / 1.8 |
| Soul | 28 | 0.81 | 0.78 | **0.62** | 1.2 / 1.4 |

Density and syncopation match, but coherence does not. Even an unrelated real beat in the same style is closer to a real bar than an independent draw is to its nearest real beat.

### 7.7 Caveats

- One kit (TD-11), ten drummers, playing to a click, mostly improvised in a requested style. Style labels come from the drummer. Hip hop drummers here may not resemble sampled breakbeats (Frane 2017).
- Quantising to the nearest sixteenth misreads strongly swung or triplet notes; swing was measured separately (section 5.1).
- The accent threshold (0.65 times the file's 95th-percentile snare velocity) is a heuristic; mid-velocity snares count as ghosts.
- Phrase position is counted from the file start, which is usually but not always a phrase start, so the bar 4 and bar 8 peaks are if anything understated.

### 7.8 Scripts

All in [`clave-subdivision-and-groove-data-scripts/`](clave-subdivision-and-groove-data-scripts/). The GMD MIDI zip is not committed: download it from https://storage.googleapis.com/magentadata/datasets/groove/groove-v1.0.0-midionly.zip (CC BY 4.0). Each runs as `python3 -I <script> <path to the unzipped groove folder>` and needs `mido`.

- `gmd_groove_stats.py`: coverage, density, per-position probability, velocity and deviation, systematic offset against jitter, swing (absolute and pairwise), consistency, anchors, change histogram, phrase position. Its output is saved in `gmd_out.txt` in the same folder.
- `velocity_ladder.py`: section 7.2.
- `marginal_vs_real.py`: section 7.6 (output in `marginal_out.txt`).
- `jazz_swing_vs_tempo.py`: jazz swing against tempo.
- `timelines_and_gmd_clave.py`: Euclidean, evenness and oddity checks, and the clave search in GMD Latin beats (output in `clave_out.txt`).

## 8. Implications for Bloodline

Everything here is **suggestion [inference]**, drawn from the findings above.

### 8.1 Three layers per groove

1. **Anchor layer (the clave).** Per style, a short named list of anchor positions per lane, plus an optional timeline: for example Boom bap kick {0, 10}, snare {4, 12}; reggaeton kick {0, 4, 8, 12}, snare {3, 6, 11, 14}; Latin, a son or rumba clave (3-2 or 2-3) as a mask or an audible cross-stick lane. Anchors are present in about 90% of bars (GMD recall 0.85 to 0.94). Allow an anchor to drop only in a phrase-end bar, with probability about 0.1. Lockable separately from the lane. Bred rarely: a new anchor set should be a "new" action, not a mutation. Store timelines as named patterns (section 2.1), with `E(k, n)` plus rotation as a generator for new ones.
2. **Body layer (the modal bar).** Each fresh groove draws **one** bar per lane: the anchors plus a few non-anchor onsets, placed by metrical weight and by agreement with the timeline (accents that land on or just before timeline strokes). Every bar repeats it. Draw the body from a **real beat's** per-position profile (a GMD-derived template), or as a whole pattern from a small curated set, instead of per step from the style's pooled odds. That alone addresses the coherence gap in section 7.6. Target onsets per bar from section 4.1: kick 3 to 5, snare accents 2 to 3, ghosts 0 to 4, hats 6 to 9 (8 for eighths plus 0 to 2 sixteenths).
3. **Variation layer.** A per-bar **change budget** against the body, shaped like the GMD histograms (section 3.2). In GMD, kick and snare accents each change by 0 in about 40% of bars and average about 1.3 to 1.5 changes per lane in ordinary bars, about 1.5 times that in bar 4, with a fill in bar 8. Those performances are improvised, and the user wants "fairly consistent", so a default of about half that (0.6 to 0.8 per lane per ordinary bar, about 1.2 in bar 4, a fill or larger change in bar 8) seems right, with the dial reaching the GMD rate about two thirds of the way up. Ghosts and hats get a larger budget (GMD about 2 to 3 per bar). Changes are mostly **additions or one-step displacements off the anchors** (Sioros-style transformations keep the note count), not removals. The dial runs from 0 (exact loop) to about 1.5 times the GMD rate.

### 8.2 Feel layer per lane

- **Beat map per lane** (section 5.4), defaulting by style: straight for modern hip hop and trap, 0.54 to 0.545 for breakbeat and boom bap (Frane's median), 0.58 to 0.60 only when chosen, triplet for shuffle and 12/8, about 0.66 for a jazz ride, and 25:33:42 as a named "jembe" feel. Kick defaults to straight even when hats swing.
- **Per-lane offsets** relative to the hats: kick -5 ms, snare backbeat -3 ms, ghosts +10 ms, hats 0. "Laid back" adds 10 to 20 ms to the snare, with the louder and longer stroke the earlier note recommends (Danielsen et al. 2015: 17.4 ms at 96 BPM).
- **Jitter**: GMD shows about 14 to 27 ms SD per hit within a performance, but the earlier note found deviations at or below performed size neutral and larger ones harmful. Default to about half of GMD (7 to 12 ms SD), structured (1/f) as the earlier note proposes, and none on the kick.
- **Velocity ladder** (section 7.2, hip hop): kick on 1 about 0.65, other kicks 0.47, backbeat 0.95, ghosts 0.27 (0.28 times the backbeat), hats 0.77, 0.51 and 0.38 for beat, & and e/a. The current values are already close; keep them.

### 8.3 Where it would touch the code

- [plugins/bloodline-breaks/core/pattern.h](../../plugins/bloodline-breaks/core/pattern.h) and [pattern.cpp](../../plugins/bloodline-breaks/core/pattern.cpp): add `anchors` (and optionally a timeline) to `StyleInfo`. Make `PatternOps::fresh` build anchors plus a body instead of per-step draws. Stop `0` digits firing by dropping the `flat` floor for non-anchor zero steps, or making it much smaller. Make `mutate` move or add one or two non-anchor hits per lane rather than reconsider every step with `spread x 0.8`. Give `hitsOf` a per-lane beat map instead of the single `swing`.
- [plugins/bloodline-breaks-2/core/loops.cpp](../../plugins/bloodline-breaks-2/core/loops.cpp): the same change in `PatternOps::fresh`; generalise `warp()` and `unwarp()` to beat maps.
- [web/bloodline-breaks-mobile-prototype.html](../../web/bloodline-breaks-mobile-prototype.html): `candidate()` and `shapedCandidate()` already repeat bar 1. Replace the per-step redraw (`0.2 + 0.4 x c`) with the per-bar change budget weighted to bars 4 and 8, and move fills to the phrase end. Revisit `defaultFeel`, which makes the snare `behind` 60% of the time, against section 5.3 (only ghosts drag in GMD).
- A small offline tool (Python, outside the plugins) could turn GMD beats into per-beat JSON templates (modal bar, anchors, per-step velocity and offset). CC BY 4.0 allows this with attribution.

## Sources not reached

All hosts below were refused by this session's proxy, so only search extracts were read. Allowing them would let a later pass check the numbers:

- `magenta.tensorflow.org` and `arxiv.org`: the GMD and E-GMD pages and Gillick et al. 2019 (the dataset facts here come from loader code and READMEs on GitHub instead).
- `osf.io`: the Drum Groove Corpora CSV (https://osf.io/3sejt/) and the IEMP collection (https://osf.io/37fws/). These are the most valuable unread datasets: commercial-record grooves (Lucerne) and Afro-Cuban, jembe and candombe onsets.
- `zenodo.org`: the candombe and IDMT-SMT-Drums records (licences and file lists).
- `mtosmt.org`: Polak 2010 and Polak and London 2014 tables (exact jembe ratios beyond 25:33:42), and Geary 2024.
- `cgm.cs.mcgill.ca` and `bridgesmathart.org`: Toussaint 2002 and 2005 full texts (the full Euclidean table; whether soukous or "rap" is in the six); Sioros and Guedes 2011.
- `ismir2015.uma.es`: Dittmar et al. 2015 ride swing tables.
- `duo.uio.no`: Câmara et al. 2020 millisecond values for laid back and pushed.
- Not found at all: a quantitative study of fill placement (measured here instead), any measurement of quintuplet or "Dilla" feel, a published per-instrument Witek weight table (taken from the GrooveToolbox code instead), the 6/8 clave transcription in Peñalosa, and a clave MIDI library with human timing.
