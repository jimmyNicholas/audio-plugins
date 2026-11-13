# Research: composing from nothing, without samples

Researched 2026-11-08. Starting point: a drive of samples and beats has died and everything on it is gone. Rather than rebuild a library, the plan is to start again with **no samples at all**: every sound is synthesised, and tools take on some of the compositional heavy lifting. Target platform for any resulting device: Max for Live on Ableton Live 11 (Max 8).

## How to read this

- **Scope:** this note avoids repeating [Boundary-pushing plugins](boundary-pushing-plugins.md), which already covers Synplant 2, Magenta Studio, Neutone, RAVE, Permut8, BYOME, Inspired by Nature, Turing Machine and Live 12 MIDI Generators. Those still apply here; this note adds the pieces a sample-free setup needs and puts them in one workflow.
- **Filter:** a tool qualifies if it makes sound without loading audio files (or needs none to be useful) **and** either pushes what an instrument can be or makes musical decisions for you.
- **Method:** web search extracts only, no full page reads. Claims from a maker's page or retailer listing are marketing copy unless a review is cited. Reasoning by the researcher is marked **[inference]**.
- **Live 11 flag:** items marked **(Live 12)** will not run on this repo's target.

## TL;DR

1. **Drums can be fully synthesised.** Microtonic generates every hit in real time with no samples, and its Patternarium evolves patterns and patches by listener vote. Live's own Drum Synths and Operator cover the free end.
2. **Free synths are now good enough to start with:** Surge XT (open source) and Vital, plus Cardinal, a free plugin build of VCV Rack with about a thousand modules, including generative ones.
3. **Let MIDI devices write the parts.** Ableton's Probability Pack (Live 11), Stochas (free), Riffer 4 and Scaler 3 carry rhythm, melody and harmony decisions. Live 11 itself has per-note chance and velocity ranges.
4. **Instruments that behave rather than just sound:** Synplant 2 (grow patches from seeds), Sumu (additive, 64 partials per voice, with vector-field spatialisation) and Dillon Bastan's Inspired by Nature (physics and nature processes, Live 11 Suite).
5. **Best first build for this repo [inference]:** a Max for Live **drum voice that also writes its own pattern**. Each hit is synthesised in Max (no samples), and the pattern evolves by keeping the variations you leave playing and discarding the ones you skip. That is Patternarium's idea, made local and inside Live.

## 1. Synthesised drums

| Tool | How it makes sound | Compositional lift | Notes |
| --- | --- | --- | --- |
| **Sonic Charge Microtonic** | Each of 8 voices is an oscillator plus noise generator, generated in real time; no samples or pre-rendered waves. All voice modules are identical, so any slot can be a kick, hat or tone. | Pattern sequencer per voice; **Patternarium**, an online evolutionary randomiser where listeners vote on which patches and rhythms survive; **BeatSpace**, an AI script that generates beats. | Around US$119 alone. Sources: [Sonic Charge](https://cdn.soniccharge.com/microtonic), [MusicRadar review](https://www.musicradar.com/reviews/tech/sonic-charge-microtonic-3-398862), [KVR Patternarium](https://www.kvraudio.com/news/sonic_charge_launches_microtonic_patternarium_and_announces_microtonic_update_14586), [Synth Anatomy BeatSpace](https://synthanatomy.com/2022/12/sonic-charge-beatspace-new-ai-based-microtonic-script-generates-beats-for-you.html). |
| **Live's Drum Synths and Operator** | Max for Live drum synth devices (kick, snare, hats, toms, clap, cymbal) and Operator's FM. | None alone; pair with section 3. | Already in Live, no cost **[inference: from Ableton's pack listings, not re-checked this session]**. |
| **Audialab Emergent Drums 2** | Generates new one-shots from trained models with no source recordings. | 16-pad instrument with layering and effects. | Strictly it outputs samples, but makes them from nothing, so it fits a fresh start. Needs internet to generate. One review of v1 found it leans "crunchy, digitally lo-fi". Sources: [Audialab](https://audialab.com/products/emergent-drums-2/), [MusicRadar](https://www.musicradar.com/news/audialab-emergent-drums-ai-samples). |

## 2. Synths and instruments

**Free foundation**

- **Surge XT:** free, open source (GPL) hybrid synth from the Surge Synth Team. VST3, AU, CLAP, LV2 and standalone on Windows, macOS and Linux. Sources: [Dubspot](https://blog.dubspot.com/plugins/surge-xt), [Bedroom Producers Blog, Vital vs Surge](https://bedroomproducersblog.com/2026/06/10/vital-vs-surge/).
- **Vital:** free tier of a visual wavetable synth; still maintained in 2026, with newer builds in early access. Source: [Bedroom Producers Blog](https://bedroomproducersblog.com/2026/06/10/vital-vs-surge/).
- **Cardinal:** free, open source plugin build of VCV Rack. Ships a fixed set of over a thousand modules from dozens of authors (Valley, Voxglitch and others), as main, Synth and FX variants. A whole generative modular patch inside one track. Sources: [Cardinal](https://cardinal.kx.studio), [about](https://cardinal.kx.studio/about).

**Boundary pushers**

- **Synplant 2:** patches grow from seeds and branch into variations; no samples needed (Genopatch, which imitates a sample, is optional). See [the earlier note](boundary-pushing-plugins.md#31-sonic-charge-synplant-2-and-genopatch).
- **Madrona Labs Sumu:** semi-modular additive synth, 64 partials per voice, with FM and spatialisation driven by vector fields. 1.0 shipped November 2024, around US$129 to US$179. Its companion app Vutu resynthesises audio, but that is optional. Sources: [Sound On Sound](https://www.soundonsound.com/news/madrona-labs-release-sumu), [Synth Anatomy](https://synthanatomy.com/2024/11/madrona-labs-sumu-novel-synth-plugin-explores-spatialized-additive-resynthesis-with-fm.html), [CDM](https://cdm.link/sumu-arrives/).
- **Inspired by Nature (Dillon Bastan and Ableton):** six instruments and effects based on natural and physical processes; Max for Live, part of Live 11 Suite. See [the earlier note](boundary-pushing-plugins.md#21-inspired-by-nature-dillon-bastan-ableton).
- **Live's physical modelling** (Collision, Tension, in Live Suite) **[inference]:** struck and bowed models give acoustic-feeling tones with no recordings.

## 3. Generators that carry the parts

| Tool | Lift | Notes |
| --- | --- | --- |
| **Ableton Probability Pack** (Live 11 Standard or later with Max for Live) | Five sequencers: Melodic Probability (chance of event, pitch and octave per step), Rhythmic Probability, Step Divider, Dr. Chaos, Probability Arp. Over 60 presets designed for them. | Was free at launch; check current pricing. Sources: [Ableton](https://www.ableton.com/en/packs/probability-pack/), [MusicRadar](https://musicradar.com/news/five-free-sequencers-come-in-the-probability-pack-for-ableton-live). |
| **Live 11 clip tools** **[inference]** | Per-note chance, velocity ranges and follow actions, built into Live 11. | Already owned. |
| **Stochas** (free, GPL) | Probabilistic polyrhythmic step sequencer and chord generator: per-step chance for trigger, timing, velocity and length; note chaining. | Source: [Bedroom Producers Blog](https://bedroomproducersblog.com/2020/08/23/stochas-open-source/). |
| **Audiomodern Riffer 4** (Oct 2026) | Five engines: hooks, basslines, arpeggios, sequences and guitar-style patterns. MIDI only. | €69, upgrade €29. Source: [Sonicstate](https://sonicstate.com/news/2026/10/02/audiomodern-releases-riffer-4/). |
| **Scaler 3** | Chord suggestions, voice leading, modulation pathways, and Motions that turn progressions into melodies, arps and basslines. | US$99. Vendor copy. Source: [Plugin Boutique](https://pluginboutique.com/product/3-Studio-Tools/93-Music-Theory-Tools/14563). |
| **TAP** (this repo) | Retunes each key as you play and sends per-note pitch over MPE. | Feeds Wavetable, Surge XT or Vital in MPE mode. |

## 4. A sample-free starting rig

Cheapest version uses only Live 11 and free plugins; paid tools are swaps, not requirements.

| Role | Free | Paid upgrade |
| --- | --- | --- |
| Drums | Live Drum Synths, Operator | Microtonic (plus Patternarium) |
| Bass and leads | Surge XT, Vital, Wavetable | Synplant 2 |
| Textures | Cardinal generative patch | Sumu, Inspired by Nature |
| Rhythm and melody | Probability Pack, Stochas, Live 11 chance | Riffer 4 |
| Harmony | Live scale tools | Scaler 3 |
| Mutation | TAP | |

**Habit worth keeping [inference]:** since nothing is a file any more, save presets and Live sets to synced or backed-up storage. A synthesised library is a folder of tiny preset files, so losing it again is easy to avoid.

## 5. Max for Live device ideas (Live 11, Max 8)

1. **Breeder drum:** one synthesised drum voice (oscillator, noise, pitch and amp envelopes, as in Microtonic's voice design) with its own step pattern. A Mutate button spawns variations; the ones you leave playing survive, the ones you skip are dropped. Patternarium's evolution, offline and per track.
2. **Kit from one seed:** a single knob or number generates a whole matched kit of synthesised drums and a starter pattern, so a session starts with a bed instead of a blank page. Seeds are tiny, so a "library" is a list of numbers.
3. **Probability voice-leader:** Melodic Probability's per-step chance, but the pitch choices follow voice leading from whatever chord is held on another track.
4. **TAP drums:** route TAP's mutations into Breeder drum so tuned percussion drifts as you play, joining the repo's devices.

Open questions:

- How convincing can a drum voice built from Max 8 MSP objects get, compared with Microtonic? Needs a quick listening spike.
- Should survival in Breeder drum be judged by time left playing, by a thumbs up and down, or by how often you return to a variation?

## Sources

- Microtonic: <https://cdn.soniccharge.com/microtonic>, <https://www.musicradar.com/reviews/tech/sonic-charge-microtonic-3-398862>, <https://www.kvraudio.com/news/sonic_charge_launches_microtonic_patternarium_and_announces_microtonic_update_14586>, <https://synthanatomy.com/2022/12/sonic-charge-beatspace-new-ai-based-microtonic-script-generates-beats-for-you.html>
- Emergent Drums 2: <https://audialab.com/products/emergent-drums-2/>, <https://www.musicradar.com/news/audialab-emergent-drums-ai-samples>
- Surge XT and Vital: <https://blog.dubspot.com/plugins/surge-xt>, <https://bedroomproducersblog.com/2026/06/10/vital-vs-surge/>
- Cardinal: <https://cardinal.kx.studio>, <https://cardinal.kx.studio/about>
- Sumu: <https://www.soundonsound.com/news/madrona-labs-release-sumu>, <https://synthanatomy.com/2024/11/madrona-labs-sumu-novel-synth-plugin-explores-spatialized-additive-resynthesis-with-fm.html>, <https://cdm.link/sumu-arrives/>
- Probability Pack: <https://www.ableton.com/en/packs/probability-pack/>, <https://musicradar.com/news/five-free-sequencers-come-in-the-probability-pack-for-ableton-live>
- Stochas: <https://bedroomproducersblog.com/2020/08/23/stochas-open-source/>
- Riffer 4: <https://sonicstate.com/news/2026/10/02/audiomodern-releases-riffer-4/>
- Scaler 3: <https://pluginboutique.com/product/3-Studio-Tools/93-Music-Theory-Tools/14563>
