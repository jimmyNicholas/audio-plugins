# Research: plugins that compose with a sample library you already own

Researched 2026-11-08. Starting point: an old hard drive of samples and beats has turned up. The question is which synth, MIDI and effect tools push boundaries **and** do some of the compositional heavy lifting, using that drive as raw material. Target platform for any resulting device: Max for Live on Ableton Live 11 (Max 8).

## How to read this

- **Scope:** this note avoids repeating [Boundary-pushing plugins](boundary-pushing-plugins.md). That note already covers Synplant 2, Magenta Studio, Neutone, RAVE, Permut8, BYOME, Inspired by Nature, Live 12 MIDI Generators and Turing Machine style tools. Here the filter is narrower: does the tool **take your own audio in** and **hand back structure** (a kit, a groove, a progression, a rearrangement)?
- **Method:** web search extracts only, no full page reads. Claims from a maker's page or retailer listing are marketing copy unless a review is cited. Reasoning by the researcher is marked **[inference]**.
- **Live 11 flag:** items marked **(Live 12)** will not run on this repo's target. They are listed because they shape what a Live 11 device could copy.

## TL;DR

1. **Map the drive first.** Timbre maps (XO, Atlas 2, Sononym, AudioStellar) turn thousands of anonymous one-shots into something you can browse by sound, then build kits and grooves from it. This is the step that makes an old library usable again.
2. **Corpus tools are the real boundary push.** FluCoMa (a free Max package) can slice, analyse and resynthesise a whole folder by audio descriptors: concatenative synthesis, where a new part is assembled grain by grain from your old beats. It runs in Max 8, so it is buildable here.
3. **Generators that respond to your material** (Riffer 4, Playbeat 4, Stochas, Scaler 3) carry the note and rhythm decisions, but most ignore what your samples sound like. That gap is the device opportunity.
4. **Stems turn finished beats back into parts.** Live 12.3 Stem Separation (Live 12 Suite) splits any file into vocals, bass, drums and other. On Live 11, an outside separator does the same job before import.
5. **Best first build for this repo [inference]:** a Max for Live device that analyses a folder from the drive with FluCoMa, places every slice on a 2D timbre map, and plays a probabilistic sequence that walks the map. That joins steps 1 to 3 in one device no listed product offers.

## 1. Map the library: timbre maps and similarity search

| Tool | What it does with your samples | Compositional lift | Notes |
| --- | --- | --- | --- |
| **XLN Audio XO** | Plots every one-shot as a dot in a 2D "Genius Space" where similar sounds cluster; swaps kit pieces for neighbours. | Built-in beat sequencer; **Playground** mode generates variations for "happy accidents". | Mature; v1.7.x updates seen on KVR. Sources: [MusicRadar review](https://www.musicradar.com/reviews/xln-audio-xo), [KVR v1.7.3](https://www.kvraudio.com/news/xln-audio-updates-addictive-drums-2-to-v2-6-0-and-xo-to-v1-7-3-63136). |
| **Algonaut Atlas 2** | AI scan and categorise of drum samples into a zoomable map. | Atlas 2 adds a drum sequencer with velocity, pattern, step size and length. | No sample layering per one review. No Atlas 3 found. Around US$99. Sources: [MusicTech review](https://musictech.net/reviews/software-instruments/algonaut-atlas-2-review/), [Sound On Sound](https://www.soundonsound.com/news/algonaut-update-atlas-ai-powered-drum-sample-manager). |
| **Sononym** | Sample browser with audio similarity search, including searching by a recorded sound or a selected region. | None directly; it is the finder, not the composer. | v1.4 adds cross-library search and duplicate finding, useful for a messy old drive. Windows, Mac, Linux. Sources: [rekkerd](https://rekkerd.org/?p=227727), [Sonicstate](https://sonicstate.com/news/2021/12/13/sononym-131). |
| **AudioStellar** | Free, open source. Builds a 2D sound map from any folder. | **Particles** roam the map and trigger what they touch; also sequence units and MIDI/OSC control. | Closest existing tool to a "the library plays itself" idea. Check the project is still active. Sources: [IRCAM forum](https://forum.ircam.fr/article/audiostellar/), [Flathub](https://flathub.org/apps/xyz.audiostellar.AudioStellar). |
| **Ableton similar-sound swap (Live 12)** | Browser finds similar sounds; Drum Rack can swap every pad for a similar sample. | Kit variation in one click. | Live 12.3 adds Splice **Search with Sound**, matching samples to a project's rhythm and harmony. Source: [Ableton new features](https://www.ableton.com/en/live/all-new-features/). |

**Why it matters for the drive [inference]:** an old drive usually means bad filenames and no tags. Every tool above ignores names and works from the audio, so the drive becomes searchable without a cleanup pass.

## 2. Corpus and concatenative tools: compose *from* the library

- **FluCoMa (Fluid Corpus Manipulation)**, free Max package, version 1.0.9 listed, Max 7 or later. Tools for slicing, decomposition (for example separating sustained from transient parts), audio-descriptor analysis, dimension reduction and machine learning over a collection of sounds. A stated project aim was hybridisation by concatenation and descriptor-based browsing. The research project ended in February 2023, but the package and its learning site remain. Sources: [Cycling '74 package](https://cycling74.com/packages/fluidcorpusmanipulation), [Cycling '74 article](https://cycling74.com/articles/content-you-need-flucoma), [CORDIS](https://cordis.europa.eu/project/id/725899).
- **Data Knot** (Rodrigo Costanzo) builds on FluCoMa with performer-oriented machine learning tools in Max. Source: [Cycling '74 article](https://cycling74.com/articles/content-you-need-flucoma).
- **What concatenative synthesis does for composition [inference]:** feed it a target (a hummed line, a drum loop, a field recording) and it rebuilds that target from the closest-matching slices of your library. The shape comes from the target, the sound comes from your old beats. It is both a sound design tool and an arranger.

## 3. Instruments that grow sounds from your samples

- **Audialab Emergent Drums 2:** generates new drum one-shots from trained models; **Deep Sampling** generates variations of your own sounds. 16-pad MIDI instrument with layering, clipping, filter and pitch. Generated sounds are stored locally; internet is only needed to generate. Price varies by seller ($37 sale to $249 list seen). One review of version 1 found kicks lean "crunchy, digitally lo-fi". Sources: [Audialab](https://audialab.com/products/emergent-drums-2/), [Native Instruments](https://www.native-instruments.com/en/products/nks-partners/audialab/emergent-drums-2/), [MusicRadar](https://www.musicradar.com/news/audialab-emergent-drums-ai-samples).
- **Output Arcade 2:** loop library you play as an instrument, with loop rearrangement per key, chromatic instruments in v2, and an AI preset generator for **your own samples**. Subscription model. Sources: [MusicRadar](https://musicradar.com/news/output-arcade-2-plugin-launched), [CDM](https://cdm.link/outputs-arcade-is-a-cloud-based-loop-library-you-play-as-an-instrument/).
- **Granulator III (Robert Henke and Ableton, Live 12):** granular instrument with MPE control of grain size, shape and position. Pairs well with TAP's per-note MPE output. Sources: [Ableton pack](https://www.ableton.com/en/packs/granulator-iii/), [Robert Henke](https://roberthenke.com/technology/granulator3.html).
- Also see **Synplant 2 Genopatch** (builds a synth patch that imitates a sample) in [the earlier note](boundary-pushing-plugins.md#31-sonic-charge-synplant-2-and-genopatch). For an old drive of one-shots, Genopatch turns any hit into a playable, evolving synth.

## 4. Generators that carry the notes and rhythm

| Tool | Lift | Notes |
| --- | --- | --- |
| **Audiomodern Riffer 4** (Oct 2026) | Five engines: hooks, basslines, arpeggios, sequences, guitar-style. MIDI only, exportable. | €69 regular, upgrade €29. Source: [Sonicstate](https://sonicstate.com/news/2026/10/02/audiomodern-releases-riffer-4/). |
| **Audiomodern Playbeat 4** | Genre-based groove generation with endless variations; audio and MIDI export. Loads your own samples. | Retailer copy only. Source: [Thomann](https://www.thomannmusic.com/audiomodern_playbeat_4.htm). |
| **Stochas** (free, GPL) | Probabilistic polyrhythmic step sequencer and chord generator: per-step chance for trigger, timing, velocity, length; note chaining. | Surge Synth Team. Source: [Bedroom Producers Blog](https://bedroomproducersblog.com/2020/08/23/stochas-open-source/). |
| **Scaler 3** | Chord suggestions, voice leading, modulation pathways, and **Motions** that turn progressions into melodies, arps and basslines. | US$99. Vendor copy. Source: [Plugin Boutique](https://pluginboutique.com/product/3-Studio-Tools/93-Music-Theory-Tools/14563). |

**The gap [inference]:** none of these listen to the samples they trigger. A groove generator does not know your snare has a long tail, and a chord tool does not know the loop you chopped is in E minor. A device that chooses notes *because of* what the audio sounds like would be new.

## 5. Stems: turn finished beats back into parts

- **Live 12.3 Stem Separation (Live 12 Suite):** splits any audio into vocals, bass, drums and other, using Music AI (Moises) algorithms. Sources: [Ableton](https://www.ableton.com/en/stem-separation-in-ableton-live/), [Electronic Groove](https://electronicgroove.com/ableton-introduces-stem-separation-in-live-12-3-update/).
- **On Live 11 [inference]:** separate outside Live first (any standalone separator), then import stems. Old finished beats on the drive become drum, bass and harmony sources for the tools in sections 1 to 4.

## 6. Suggested path for the drive

1. **Triage:** run Sononym (or XO for drums) over the drive. Find duplicates, find the good material.
2. **Split:** separate stems from finished beats you want to reuse.
3. **Kit and groove:** build kits in XO or Atlas 2 and let their sequencers propose patterns; or load the kits into Playbeat 4.
4. **Harmony:** detect or choose a key, sketch progressions in Scaler 3, route its Motions or Riffer 4 into a synth made with Synplant Genopatch from one of your own hits.
5. **Go stranger:** AudioStellar particles, or FluCoMa concatenation, to let the library rearrange itself.

## 7. Max for Live device ideas (Live 11, Max 8)

1. **Crate Map [inference]:** pick a folder; FluCoMa slices onsets, analyses MFCCs and loudness, and reduces to a 2D map in the device UI. Click to audition. This is the foundation for the next three.
2. **Map Walker:** a playhead moves across Crate Map with Stochas-style probability per step and a Turing Machine style lock knob, so a pattern can repeat, drift or mutate. Like AudioStellar's particles, but clock-synced inside Live.
3. **Rebuild:** concatenative resynthesis. Drop in a target clip; the device rebuilds it from the nearest slices in your library. A Mix knob blends original and rebuild.
4. **Listening Groove:** a drum pattern generator that reads each sample's length and brightness before placing it, so long tails get space and bright hits avoid clashing. Answers the gap in section 4.
5. **TAP link:** Map Walker outputs MIDI that can feed TAP, so the notes mutate on the way to Granulator-style or Simpler instruments, joining this repo's two devices.

Open questions:

- How large a folder can FluCoMa analyse in Max 8 inside Live before load time hurts? Needs a spike.
- Should analysis results be cached next to the samples (a sidecar file) so the map loads instantly next session?
- Does the device save the map in the Live set or point at the drive, and what happens when the drive is unplugged?

## Sources

- XO: <https://www.musicradar.com/reviews/xln-audio-xo>, <https://www.kvraudio.com/news/xln-audio-updates-addictive-drums-2-to-v2-6-0-and-xo-to-v1-7-3-63136>
- Atlas 2: <https://musictech.net/reviews/software-instruments/algonaut-atlas-2-review/>, <https://www.soundonsound.com/news/algonaut-update-atlas-ai-powered-drum-sample-manager>
- Sononym: <https://rekkerd.org/?p=227727>, <https://sonicstate.com/news/2021/12/13/sononym-131>
- AudioStellar: <https://forum.ircam.fr/article/audiostellar/>, <https://flathub.org/apps/xyz.audiostellar.AudioStellar>
- Ableton: <https://www.ableton.com/en/live/all-new-features/>, <https://www.ableton.com/en/stem-separation-in-ableton-live/>, <https://electronicgroove.com/ableton-introduces-stem-separation-in-live-12-3-update/>, <https://www.ableton.com/en/packs/granulator-iii/>, <https://roberthenke.com/technology/granulator3.html>
- FluCoMa: <https://cycling74.com/packages/fluidcorpusmanipulation>, <https://cycling74.com/articles/content-you-need-flucoma>, <https://cordis.europa.eu/project/id/725899>
- Emergent Drums 2: <https://audialab.com/products/emergent-drums-2/>, <https://www.native-instruments.com/en/products/nks-partners/audialab/emergent-drums-2/>, <https://www.musicradar.com/news/audialab-emergent-drums-ai-samples>
- Arcade: <https://musicradar.com/news/output-arcade-2-plugin-launched>, <https://cdm.link/outputs-arcade-is-a-cloud-based-loop-library-you-play-as-an-instrument/>
- Riffer 4 and Playbeat 4: <https://sonicstate.com/news/2026/10/02/audiomodern-releases-riffer-4/>, <https://www.thomannmusic.com/audiomodern_playbeat_4.htm>
- Stochas: <https://bedroomproducersblog.com/2020/08/23/stochas-open-source/>
- Scaler 3: <https://pluginboutique.com/product/3-Studio-Tools/93-Music-Theory-Tools/14563>
