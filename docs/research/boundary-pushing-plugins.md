# Research: audio and MIDI tools that push the boundaries of creative practice

Researched 2026-11-01. Purpose: a reference for future device ideas in this repo, framed by TAP ("The Annoying Piano", [`devices/tap/docs/SPEC.md`](../../devices/tap/docs/SPEC.md)). Target platform for any resulting device: Max for Live on Ableton Live 11 (Max 8).

## How to read this

- **Framing:** TAP is a stateful MIDI effect. Every key you release changes the stored pitch of keys you play later (Swap, Gravity, Move, Repel, Echo, Rotate, Wander, Harmony), so the instrument changes because of what you played. This doc looks for other tools whose design ideas are of that kind: the tool has memory, reacts to context, mutates, or changes what "playing" means. It is not a list of the best-sounding plug-ins.
- **Method caveat:** the sandbox could not fetch any page directly. Every maker site tried (`oddsound.com`, `pivotuner.com`, `support.apple.com`, `isartum.net`) failed DNS, `arxiv.org` was refused by the egress proxy, and GitHub API access to public repos (RAVE, nn~, DDSP-VST, Neutone, MTS-ESP) was not enabled for the session. So **every claim below comes from search-engine extracts of the cited page, not a full read**, and is marked **[extract]**. Reasoning by the researcher is marked **[inference]**. Where an extract came from a maker's own page, manual, paper, repo or app-store listing, that URL is cited. Where only press or retailer coverage was found, that is said.
- **Source quality:** about half of the products have a maker-owned source behind the extract (Ableton pack pages and blog, Apple help pages, Magenta and Qosmo sites, the RAVE repo and PyPI page, Cycling '74 docs, midi.org, the DataMind manual, Sonic Charge's Permut8 page, Plugin Alliance, Native Instruments, Apple App Store listings written by the developer). The rest (Pivotuner pricing, Synplant 2 details, Madrona Labs, Glitchmachines, Expressive E, MTS-ESP specifics) rest on press, forum or retailer pages, because the maker's own page did not appear in search results or could not be read.
- **Status checks:** "current" means a maker or store page in the results still lists the product. Prices change; treat them as indicative, dated by their source.
- **Confidence:** High = a maker source plus at least one independent source agree. Medium = one maker source, or several secondary sources. Low = a single secondary source, or inference.

## TL;DR

| Tool | Category | The boundary-pushing idea | Status (as found) | Confidence |
|------|----------|---------------------------|-------------------|------------|
| Hermode Tuning (Logic Pro) | Adaptive tuning | Retunes each chord towards pure intervals while keeping the total drift from equal temperament at zero | Current, built into Logic (Mac and iPad) | High |
| Pivotuner | Adaptive tuning | Real-time pure intonation that pivots on shared notes, so the tonal centre can drift microtonally on purpose | Sold (VST3/AU), described as a side project | Medium |
| ODDSound MTS-ESP | Tuning infrastructure | One master plug-in sets the tuning of every client plug-in in a session, and can change it live | Suite paid, Mini free, open client library | Medium |
| Leimma and Apotome | Tuning exploration, generative | Free browser tools for non-Western and custom tunings, with a generative player | Free, browser; current status unverified | Medium |
| Infinitone 2 | Adaptive tuning | Dynamic per-track microtuning morphs within a song | Sold (VST3/AU/standalone) | Low |
| Live 12 Tuning Systems, Microtuner | Tuning in the DAW | Tuning as a property of the whole Live Set, Scala import, MTS-ESP link | Live 12 feature; Microtuner free with Live 11.1+ originally, now in Live 12 Suite | High |
| Inspired by Nature (Dillon Bastan) | Physics-based devices | Simulation plus visualisation plus audio process: balls, particles and growing trees play the notes | Free, ships with Live 12 Suite (launched with Live 11) | High |
| Turing Machine and its ports | Stateful generative | A shift register loop that "slips": locked, random, or occasionally mutating | Hardware by Music Thing Modular; free VCV and Max for Live ports | Medium |
| Mutable Instruments Marbles | Stateful generative | "Déjà vu" knob: one control from fully random to a locked loop | Module discontinued; clones exist | Medium |
| Fugue Machine (Classic, Rubato) | Generative MIDI | Several playheads read one piano roll at different speeds, directions and pitches; Rubato bends time per playhead | Current (iOS AUv3, Rubato also Mac) | High |
| Rozeta and Mozaic (Bram Bos) | Generative and scriptable MIDI | AUv3 MIDI plug-ins as small composable generators; Mozaic lets users script their own | Current (iOS) | High |
| Live 12 MIDI Transformations and Generators | Clip-level generative | Generative tools in the clip editor, extendable with Max for Live | Live 12 only | High |
| Synplant 2 (Genopatch) | Evolutionary, ML | Sounds as seeds with mutating branches; Genopatch grows a patch from an audio sample | Current, $149 | Medium |
| Magenta Studio | ML MIDI | Neural networks continue, interpolate, generate and humanise MIDI clips inside Live | Free; requires Live 10.1+; maintenance status unclear | Medium |
| Neutone FX and Morpho | ML audio | Host for real-time neural timbre models (RAVE, DDSP); Morpho adds paid custom training | FX free; Morpho $99, manual updated June 2026 | Medium |
| IRCAM RAVE and nn~ | ML audio | Train your own real-time timbre model; run it in Max or Pure Data | Open source, v2 series active | High |
| DDSP-VST | ML audio | Timbre transfer plug-in with a free web trainer for your own instrument | Free | Medium |
| DataMind Combobulator | ML audio | Artist-trained "Artist Brains" with a revenue share to the artists | Current, about $149 | Medium |
| Sonic Charge Permut8 | Destructive, programmable | A 12-bit variable-rate delay whose read position is driven by tiny "operator" instructions | Maker page still live; update status unverified | Medium |
| Unfiltered Audio BYOME, SpecOps | Destructive, modular | Build-your-own effect chains with per-module randomisation; spectral bin mangling | Current (Plugin Alliance, subscription) | Medium |
| Glitchmachines Fracture, Hysteresis, Quadrant | Destructive | Buffer and delay "malfunctions"; modular patching in Quadrant | Fracture and Hysteresis free; Quadrant paid | Low |
| Native Instruments Flesh (Tim Exile) | Performance sampler | Turns a loaded sample into a playable multi-engine performance from a keyboard | Still listed by NI (Reaktor 6) | Medium |
| Madrona Labs Sumu, Kaivo | Novel synthesis | Sumu: 64 partials patched like a modular; Kaivo: granular excitation into physical resonators | Current | Medium |
| Expressive E Osmose, ROLI Seaboard, MPE | Expressive control | Per-note pitch, pressure and slide; MPE gives each note its own MIDI channel | MPE spec v1.1 plus MIDI 2.0 profile; Osmose company status unverified; ROLI now under Luminary | High (MPE) / Low (company status) |
| Orca, Strudel | Browser and terminal live coding | Sequencers written as grids of letters (Orca) or pattern code (Strudel) | Free, open source | Medium |
| Bloom: 10 Worlds | Generative app | Tap to seed a generative piece that keeps evolving without you | Current (iOS, Android) | Medium |

**Main finding [inference]:** the tools that really change practice share a short list of moves: (1) the instrument keeps state and lets the past change the future (TAP, Turing Machine, Marbles, Hermode, Pivotuner); (2) a single control spans "locked" to "chaotic" (Marbles, Turing Machine, Synplant branches); (3) a simulation, not a knob, drives the parameters (Bastan); (4) the user gets a tiny language instead of a fixed effect (Permut8, Mozaic, Orca, Strudel); (5) tuning becomes a live, shared property of the session rather than a fixed table (MTS-ESP, Live 12 Tuning Systems); (6) a learned model replaces a hand-built sound engine (RAVE, Neutone, Synplant Genopatch). Section 8 turns these into device ideas.

## 1. Adaptive and dynamic tuning

These are TAP's nearest relatives: the pitch of a key depends on context, not on a fixed table.

### 1.1 Hermode Tuning in Logic Pro

- Apple: Hermode Tuning (HMT) "automatically controls the tuning of electronic keyboard instruments (or the Logic Pro for Mac software instruments) during a musical performance". It analyses the position of each note in a chord and "zeroes the sum of each note's distance to the tempered scale". Tuning "is continuously adjusted in accordance with the musical content". **[extract]** https://support.apple.com/ar-qa/guide/logicpro/lgcpa88a63e7/mac, https://help.apple.com/logicpro/mac/10.1/en.lproj/lgcpa88a63e7.html
- Fifths can be tuned to 3:2 and major thirds to 5:4. An older help page states "Hermode Tuning is dynamic, not static". **[extract]** same Apple pages; https://help.apple.com/logicpro/mac/9.1.6/en/logicpro/usermanual/chapter_43_section_6.html
- Modes in the iPad guide: Classic (3/5-All), Baroque (3/5-Adaptive: central chords tuned purely, distant chords less so) and Pop/Jazz (3rds, 5ths and 7ths, used at 90 to 100% Depth). Depth runs 0 to 100%. **[extract]** https://support.apple.com/guide/logicpro-ipad/lpip67dc8147/ipados
- Criticism: the Pivotuner paper says HMT "is not very customizable". **[extract]** https://arxiv.org/pdf/2306.03873
- **Status:** current, built into Logic Pro for Mac and iPad. **Confidence:** High.
- **Why it matters for TAP [inference]:** the zero-sum constraint is a neat idea TAP lacks. TAP's keys can drift without limit; Hermode drifts per chord but keeps the average anchored to equal temperament, so the instrument never wanders off. That is a candidate "anchor" option for TAP modes.

### 1.2 Pivotuner

- A plug-in by Dmitri Volkov that "tunes MIDI data in pure intonation in real time". Its approach rests on "pivot notes" shared between purely tuned chords, and it is designed to allow microtonal modulation, which many adaptive tuners avoid. Keys can be locked, "which prevents microtonal modulation and enables unusual sonorities". **[extract]** https://www.kvraudio.com/product/pivotuner-by-dmitri-volkov, https://midi.org/?p=2041
- The paper's conclusion: Pivotuner can "consistently and reliably perform microtonal modulations", for example in live performance. Acknowledgements credit Ben Bloomberg and Jacob Collier. **[extract]** https://arxiv.org/pdf/2306.03873
- Formats AU and VST3, macOS and Windows; sold as a lifetime licence on Gumroad (prices vary across listings: $47 sale, or $5 a month). The developer calls it a side project. **[extract]** https://sporkl.gumroad.com/l/hsvkby, https://www.kvraudio.com/product/pivotuner-by-dmitri-volkov
- **Status:** sold; maker site `pivotuner.com` could not be reached. **Confidence:** Medium.
- **Why it matters for TAP [inference]:** Pivotuner treats drift as a feature you can steer, which is exactly TAP's stance. Its "lock" control is a useful counterweight: freeze some keys while others keep moving.

### 1.3 ODDSound MTS-ESP

- A master/client system: "a single master plugin [can] simultaneously control the tuning of any number of connected client plugins across a DAW session", and connection is automatic. Tunings can change in real time. **[extract]** https://cdm.link/inside-mts-esp-the-new-tuning-tool-from-oddsound-and-aphex-twin/, https://musictech.com/news/aphex-twin-and-oddsound-mts-esp-plug-in-musical-tuning/
- Suite includes the master, an editor and an MTS-ESP MIDI Client that sends tuning to hardware via MIDI Tuning Standard or MPE. A free MTS-ESP Mini imports `.scl`/`.kbm`/`.tun` files. **[extract]** https://www.gearnews.com/oddsound-mts-esp-suite-set-your-tuning-free-at-last, https://equipboard.com/items/oddsound-mts-esp-suite
- The client library is open (BSD Zero licence per a community repo), there is a JUCE wrapper, Surge can act as master, and a free open-source master exists. Clients load a shared library from a fixed path (`/Library/Application Support/MTS-ESP/libMTS.dylib` on macOS). **[extract]** https://github.com/baconpaul/simple-mts-main, https://marketplace.juce.com/products/juce_mts_esp
- Developed with Aphex Twin. **[extract]** https://crackmagazine.net/2021/03/aphex-twin-collaborates-with-uk-startup-on-microtuning-synth-plug-in/
- **Status:** current (maker site unreachable; status from 2021 to 2024 coverage and active community projects). **Confidence:** Medium.
- **Why it matters [inference]:** tuning becomes shared session state. A future TAP that published its live pitch table as an MTS-ESP master could retune any MTS-ESP synth without MPE. Max 8's `js` cannot load `libMTS` directly, so this would need an external or an existing bridge (see 1.5).

### 1.4 Leimma and Apotome (Khyam Allami with Counterpoint)

- Free browser tools launched at CTM Festival 2021. Leimma explores, creates, hears and plays microtonal tuning systems and exports Scala files and MPE; Apotome is a generative music environment built on octave-repeating tunings and their scales and modes. **[extract]** https://sevish.com/2021/leimma-apotome-two-new-web-tools-for-microtonal-music/, https://cdm.link/escaping-tunings-restrictive-default-setting-a-conversation-with-khyam-allami/
- First-party interview: Allami frames 12-tone equal temperament as a "restrictive default setting" in music software. **[extract]** https://cdm.link/escaping-tunings-restrictive-default-setting-a-conversation-with-khyam-allami/, https://redef.com/item/603806c52a569f07a118c57c
- **Status:** free; the official site (`isartum.net`) could not be reached, so current availability is unverified. **Confidence:** Medium.
- **Why it matters [inference]:** making tuning the first thing you choose, and putting it in a browser at no cost, is a cultural boundary push, not just a technical one.

### 1.5 Live 12 Tuning Systems and Ableton Microtuner

- Live 12 supports `.scl` and its own `.ascl` extension; a tuning loaded in the Tuning section is saved with the Set and the piano roll shows the tuning's note names. Built-in instruments, MPE plug-ins and Max for Live instruments are supported "provided that their pitch bend range is set to 48 semitones". **[extract]** https://www.ableton.com/de/live-manual/12/using-tuning-systems/
- Microtuner: a free Max for Live MIDI device released in 2022 for Live 11.1+, now included with Live 12 Suite. Up to 128 pitches, a scale generator, two scale decks blendable in real time, Lead/Follow sync between instances, and a link to MTS-ESP clients. Later versions export `.scl`. **[extract]** https://www.ableton.com/en/packs/microtuner/, https://musictech.com/news/gear/ableton-debuts-free-downloadable-midi-device-microtuner
- **Status:** current. **Confidence:** High.
- **Why it matters for TAP [inference]:** Microtuner proves a Max for Live device on Live 11 can blend two tunings in real time and talk to MTS-ESP, so the bridge in 1.3 is possible on TAP's platform. Live 12 also uses the same 48-semitone bend range TAP assumes.

### 1.6 Infinitone

- Infinitone DMT (2021) is sold as dynamic microtuning across tracks with morphing; its founder contrasts this with MTS-ESP's single session-wide tuning. Infinitone 2 adds a built-in synth and real-time retuning of third-party synths via MPE, MTS-ESP or multitimbral MIDI. **[extract]** https://giveitaspin.gr/2021/10/introducing-freedom-of-pitch-infinitone-dmt.html, https://www.kvraudio.com/marketplace/infinitone-2-by-infinitone
- **Status:** Infinitone 2 on sale. Sources are vendor-led. **Confidence:** Low.

### 1.7 Background: adaptive tuning research

- Sethares' adaptive tuning work is an earlier academic implementation of context-dependent retuning. **[extract]** https://sethares.engr.wisc.edu/html/adaptun.html
- Stange, Wick and Hinrichsen describe a dynamically adapting just intonation scheme. **[extract]** https://arxiv.org/pdf/1706.04338
- Xenharmonic Wiki definition of adaptive just intonation: predefined ratios "over a floating tonal center that is defined by the most recent context". **[extract]** https://en.xen.wiki/w/Adaptive_JI

## 2. Stateful, generative and physics-based MIDI

### 2.1 Inspired by Nature (Dillon Bastan, Ableton)

- Free pack of seven Max for Live devices and 214 presets, made with Ableton; now requires Live 12 and ships with Live 12 Suite (it launched in 2021 with Live 11 Suite). **[extract]** https://www.ableton.com/en/packs/inspired-nature/, https://www.attackmagazine.com/technique/tutorials/getting-freaky-with-live-11s-new-inspired-by-nature-devices/
- **Bouncy Notes:** a gravity-based MIDI sequencer. Played notes drop balls onto the matching keys and the bounces generate new notes; it can be set to the project key and a limited range. **Tree Tone:** each branch of a growing tree is a resonator with its own frequency, decay and amplitude. **Vector** devices (Vector Map, Vector FM, Vector Grain, Vector Delay): each particle is a voice, grain or delay line, and particle motion can be mapped to any automatable parameter. **Emit:** particles travel through a sample using phase-vocoder playback. **[extract]** https://www.ableton.com/en/packs/inspired-nature/, https://www.ableton.com/fr/blog/world-of-sound-explore-live-11s-inspired-by-nature-devices
- First-party interview: Bastan started on Lemur, moved to Max, and drew on Daniel Shiffman's *The Nature of Code*. His devices "use ripples, fractals or particles to manipulate sound rather than traditional sliders, knobs and piano rolls" and generally have three parts: "a simulation, a visualization and an audio process". **[extract]** https://www.ableton.com/fr/blog/dillon-bastan-water-ripples-and-sound-waves
- **Status:** current, free. **Confidence:** High.
- **Why it matters for TAP [inference]:** Bouncy Notes is a MIDI effect where input sets a physical system going and the system plays back, which is close to TAP's Gravity and Repel modes. Bastan's simulation, visualisation, audio triad is a ready template: TAP's pitch table drawn as masses on springs would make its state legible.

### 2.2 Turing Machine and its software ports

- The Music Thing Modular Turing Machine (designer Tom Whitwell) is "a binary sequencer, based around a 16 bit memory circuit called a shift register". At the knob's extremes it loops; near 3 or 9 o'clock it "slips", looping but occasionally changing notes. Whitwell notes it is not a real Turing machine. **[extract]** https://cdm.link/newswires/tom-whitwell-on-turing-machine-and-how-to-design-your-first-module/
- Free VCV Rack versions (with Volts and Pulses expanders) by Stellare Modular, recreated from the Mk2 schematics. **[extract]** https://github.com/stellare-modular/vcv-rack, https://www.synthtopia.com/content/2017/12/01/free-vcv-rack-software-modular-synth-gets-ableton-link-turing-machine-modules/
- A free Max for Live "Turing Machine" MIDI device by mindlesstrx, with three engines and a mapper to other tracks. **[extract]** https://mindlesstrx.gumroad.com/l/turing
- **Status:** ports free; Live version compatibility of the Max for Live port unverified. **Confidence:** Medium.

### 2.3 Mutable Instruments Marbles

- Official manual: whenever the module needs a random choice "it queries the DEJA VU section". The control runs from fully random to a locked loop, with loop lengths set by the knob. **[extract]** https://pichenette.readthedocs.io/en/latest/modules/marbles/manual/
- Déjà vu can be enabled separately for triggers and voltages. **[extract]** https://www.soundonsound.com/reviews/mutable-instruments-marbles
- **Status:** the module is discontinued; clones are sold. **[extract]** https://modulargrid.net/e/before-void-marbles-clone **Confidence:** Medium.
- **Why it matters for TAP [inference]:** one knob that spans "remember" to "forget" is the clearest single control for any stateful device. TAP's Wander mode has randomness but no memory of its random choices.

### 2.4 Fugue Machine (Alexandernaut)

- "The world's first multi-playhead piano roll": up to four playheads read one sequence at different speeds, directions and pitches. Classic $19.99 (iOS, AUv3 MIDI, Ableton Link). Rubato $39.99 adds time-bending playheads that accelerate, decelerate, freeze and reverse, per-playhead arpeggiation and parameter automation, and runs on Mac too. **[extract]** https://apps.apple.com/app/id1014191410, https://apps.apple.com/app/id6670253122, https://alexandernaut.com/fuguemachine/
- **Status:** current. **Confidence:** High.

### 2.5 Rozeta and Mozaic (Bram Bos)

- Rozeta is a suite of six AUv3 MIDI sequencer plug-ins that need a host with AU MIDI output (AUM was the main example at launch). CDM credits Rozeta and Bos's write-up with paving the way for MIDI processing in AUv3. **[extract]** https://www.soundonsound.com/reviews/bram-bos-rozeta-sequencer-suite, https://cdm.link/mozaic-scriptable-midi-ipad-iphone/
- Mozaic is "a scriptable tool for building your own MIDI plugins" with LFOs, pre-made GUI layouts and scales; it runs in AUv3 MIDI hosts and makes no sound itself. **[extract]** https://apps.apple.com/us/app/mozaic-plugin-workshop/id1457962653
- **Status:** both on the App Store. **Confidence:** High.
- **Why it matters [inference]:** Mozaic shows that giving users a small scripting layer creates a community of tools. TAP's ES5 `js` engine is already a script; exposing a user "mode slot" is the Max equivalent.

### 2.6 Ableton Live 12 MIDI Transformations and Generators

- Live 12 added Generators (new MIDI from constraints) and Transformations (changes to existing notes) in the clip view; most generators are pseudo-random, "no AI here". Available in Standard and Suite. **[extract]** https://ableton.com/en/live/all-new-features, https://www.soundonsound.com/techniques/ableton-live-12-midi-generators, https://musicradar.com/how-to/live-12-midi-tools
- Max for Live can create new ones with `live.miditool.in` and `live.miditool.out`: a Transformation's output replaces the clip's notes, a Generator's is added. Contextual data such as grid and scale is passed in. **Live 12 and later only.** **[extract]** https://docs.cycling74.com/userguide/m4l/live_miditools/, https://docs.cycling74.com/reference/live.miditool.out
- Ableton's MIDI Tools pack (Philip Meyer, five devices) needs Live 12.1. **[extract]** https://www.ableton.com/en/packs/midi-tools
- **Status:** current. **Confidence:** High.
- **Gap for this repo [inference]:** none of this runs on Live 11, the repo's target. A clip-level TAP ("apply the last N releases' drift to this clip") would need Live 12.

## 3. Evolutionary and machine-learning instruments

### 3.1 Sonic Charge Synplant 2 and Genopatch

- The interface is a seed with 12 branches of variations; growing a branch moves the sound away from the seed, planting a branch makes it the new seed. Bulb modes trigger branches by velocity, key range or in layers. A DNA editor exposes the seed's genes. **[extract]** https://cdm.link/synplant-2-hands-on/, https://synthanatomy.com/2023/10/sonic-charge-synplant-2-popular-synth-plugin-gets-a-genetic-sound-design-boost.html
- Genopatch: load a sample, select up to two seconds, and it searches for synth settings that match, locally, without a cloud service. CPU heavy; a single short note will not give a fully playable patch. **[extract]** https://www.soundonsound.com/reviews/sonic-charge-synplant-2, https://musictech.com/reviews/plug-ins/sonic-charges-synplant-2-review/
- $149, $50 upgrade from Synplant 1; launched October 2023. **[extract]** https://synthanatomy.com/2023/10/sonic-charge-synplant-2-popular-synth-plugin-gets-a-genetic-sound-design-boost.html
- **Status:** current (maker page not in results). **Confidence:** Medium.
- **Why it matters for TAP [inference]:** "seed and branches" is a user interface for exploring variation without parameters. TAP's pitch table could be planted as a seed, with branches as alternative drifted tables.

### 3.2 Magenta Studio (Google)

- Five tools built on Magenta's open models: Generate (MusicVAE, 4-bar phrases), Continue (RNN continuation), Interpolate (blends two clips), Groove (humanises timing and velocity) and Drumify (drums from a rhythm). Available standalone and as Max for Live devices; Max bridges to an Electron app because Electron cannot read or write Live clips. Requires Live 10.1 and Max for Live. **[extract]** https://magenta.tensorflow.org/studio-announce, https://www.ableton.com/en/blog/magenta-studio-free-ai-tools-ableton-live, https://github.com/tensorflow/magenta-studio
- **Status:** free. Magenta's site now hosts Studio under a `/v1/` path, which suggests the original version is archived rather than developed **[inference]**. No "2.0" release was found. **Confidence:** Medium.

### 3.3 Neutone FX and Neutone Morpho (Qosmo)

- Neutone FX is a VST3/AU host for real-time neural models; timbre-transfer models are RAVE and DDSP, steered through their latent space. **[extract]** https://qosmo.notion.site/Getting-started-607e99436b1243b5a6d273a76ee4811a, https://qosmo.jp/en/news/neutone
- Morpho resynthesises input into the timbre of a trained model while keeping its temporal shape; free to try with four models, $99 for the full version; custom models via paid training tokens; artists can sell models through the "Cocoon" programme. Manual versions run from 1.1.0 (October 2024) to a June 2026 update. **[extract]** https://sonicstate.com/news/2024/04/26/neutone-morpho-v1-released/, https://neutone.supabase.co/storage/v1/object/public/morpho-public/manual/2026-06-25_NeutoneMorpho_Manual.pdf, https://audiocipher.com/post/neutone
- Project LYDIA with Roland previews a hardware effect running user-trained Morpho models. **[extract]** https://qosmo.jp/en/news/neutone-project-lydia
- A 2025 Qosmo/Neutone paper notes only DDSP and RAVE have seen practical use, due to training stability and real-time efficiency. **[extract]** https://webdev.apsipa.org/proceedings/2025/papers/APSIPA2025_P071.pdf
- **Status:** active. **Confidence:** Medium.

### 3.4 IRCAM RAVE and nn~

- RAVE is "a Realtime Audio Variational autoEncoder" from IRCAM's ACIDS team, installed with `pip install acids-rave`; v2 is current and PyPI shows 2.3.x releases. Models export to TorchScript with a `--streaming` flag for Max or Pure Data (without it, Max gives clicks). Models load in the `nn~` external; it can run on a Raspberry Pi 4 via Pure Data. A RAVE VST beta is listed. **[extract]** https://github.com/acids-ircam/RAVE, https://pypi.org/project/acids-rave
- **Status:** open source, active. **Confidence:** High.
- **Relevance [inference]:** `nn~` is the only route in this list that puts a user-trained neural model inside Max itself. Whether current `nn~` builds still load in Max 8 under Live 11 was not checked.

### 3.5 DDSP-VST (Magenta)

- A timbre-transfer plug-in; a free web trainer makes a model "with as little as a couple minutes of audio" and a few hours on a free Colab GPU. **[extract]** https://magenta.tensorflow.org/ddsp-vst, https://magenta.tensorflow.org/ddsp-vst-blog
- **Status:** free; activity level not checked. **Confidence:** Medium.

### 3.6 DataMind Audio Combobulator

- Resynthesises incoming audio through "Artist Brains" trained with the artists' involvement; artists receive 50% of sales. Scale and Offset choose how widely and where in the network timbres are drawn. VST/AU/AAX. The manual says it "will not make music for you" and the models are not meant to replicate the artists. **[extract]** https://datamindaudio.ai/wp-content/uploads/2024/01/Combobulator-User-Manual-v0.9.2.pdf, https://www.musicradar.com/news/datamind-audio-combobulator
- Native Instruments lists it at $149; a Windows version followed the Mac launch. **[extract]** https://www.native-instruments.com/en/products/nks-partners/datamind-audio/combobulator/, https://www.recordoftheday.com/news-and-press/datamind-audio-launches-long-awaited-virtual-ai-instrument-and-ethically-sourced-artist-brains-for-windows-users
- **Status:** current. **Confidence:** Medium.
- **Why it matters [inference]:** its boundary push is as much the consent and payment model as the DSP.

## 4. Destructive and unstable effects

### 4.1 Sonic Charge Permut8

- Sonic Charge: "a 12-bit digital delay with variable sample rate from 0 to 352 kHz" is at its centre. **[extract]** https://cdn.soniccharge.com/permut8
- Two "instructions" run in series, each choosing one of four operators with a pair of 8-bit operands, to manipulate the delay's read position. Eleven program banks, 320 presets, from delays and flangers to beat repeaters. Switches can be played from MIDI notes. **[extract]** https://www.soundonsound.com/node/4906681, https://www.attackmagazine.com/reviews/gear-software/sonic-charge-permut8/
- **Status:** maker page live; released 2012; recent updates not verified. **Confidence:** Medium.
- **Why it matters [inference]:** a deliberately primitive instruction set, shown as binary switches, turns an effect into a tiny machine you program by feel. That is a model for a "programmable mode" in TAP.

### 4.2 Unfiltered Audio BYOME and SpecOps

- BYOME ("Build Your Own Modular Effect"): over 40 modules, over 400 presets, randomisation per section, per effect and per modulator; included in Plugin Alliance CORE and PRO subscriptions. **[extract]** https://www.plugin-alliance.com/products/byome
- SpecOps splits audio into thousands of frequency bins and offers 36 spectral effects, with a 16-step sequencer and modulators. **[extract]** https://www.sweetwater.com/store/detail/SpecOps--unfiltered-audio-specops, https://www.fast-and-wide.com/equipment-releases/outboard/10270-unfiltered-audio-specops
- **Status:** current. **Confidence:** Medium.

### 4.3 Glitchmachines

- Fracture is a free buffer effect "geared toward creating robotic artifacts and abstract musical malfunctions"; Hysteresis is a free glitch delay (v1.3: AU and VST3, 64-bit); Quadrant is a paid modular processor with patch cables (2.0 added 14 modules); Fracture XT is the paid patchable sibling. **[extract]** https://sonicstate.com/news/2014/02/03/free-plug-in-creates-robotic-artifacts-/, https://www.synthtopia.com/content/2014/06/30/free-glitch-delay-plugin-for-mac-windows-glitchmachines-hysteresis/, https://synthanatomy.com/?p=85427
- **Status:** current availability not verified on the maker site. **Confidence:** Low.

### 4.4 Native Instruments Flesh (Tim Exile)

- A Reaktor 6 instrument (runs in the free Reaktor Player) released November 2015 at $99; sample, monosynth, polysynth and sub engines turn a loaded sample into a keyboard performance; designed with Kontrol S-Series keyboards in mind. **[extract]** https://www.native-instruments.com/en/products/komplete/synths/flesh/, https://cdm.link/flesh-from-tim-exile-transforms-sounds-into-performances/
- **Status:** still listed by NI; no discontinuation found; update status unknown. **Confidence:** Medium.

## 5. Novel synthesis, physical modelling and expressive interfaces

### 5.1 Madrona Labs Sumu and Kaivo

- Sumu (2024): sounds built from up to 64 partials, each with frequency, level and noisiness over time, patched like a modular; the free companion app Vutu analyses samples into partials files. Also FM, a Space module and a ladder filter. Regular price $179. **[extract]** https://cdm.link/2024/05/madrona-labs-sumu-hands-on/, https://rekkerd.org/madrona-labs-launches-sumu-software-synthesizer
- Kaivo: granular synthesis feeding physical-model resonators (a polyphonic resonator with string, chime and spring models, and a body resonator). MPE support since Kaivo 1.2; all instruments at 1.9.5 with Apple Silicon support. **[extract]** https://kvraudio.com/news/madrona-labs-updates-its-synthesizer-lineup-46868, https://synthtopia.com/content/2015/07/30/aalto-1-7-brings-support-for-multidimensional-polyphonic-expression
- **Status:** current. **Confidence:** Medium (no maker page in results).

### 5.2 Expressive E Osmose, ROLI Seaboard

- Osmose: a keyboard where each key senses velocity, pressure and sideways movement, with a 24-voice Haken EaganMatrix engine; past a pressure threshold a new sound variation can trigger. **[extract]** https://www.sweetwater.com/store/detail/Osmose, https://kraftmusic.com/products/expressive-e-osmose-61-key-synthesizer-mpe-midi-controller
- Expressive E's 2025 to 2026 company status could not be confirmed. **[extract, absence]**
- ROLI went into administration in 2021 and was taken over by Luminary, which kept the ROLI name; Seaboard RISE 2 launched in 2022. **[extract]** https://roli.com/blog/roli-reforms-as-luminary-what-you-need-to-know, https://cdm.link/roli-hits-reboot-files-for-administration-as-it-shifts-to-beginners-and-subscriptions/
- **Confidence:** Medium (products), Low (company status).

### 5.3 MPE (MIDI Polyphonic Expression)

- Each sounding note is temporarily given its own MIDI channel so pitch bend and controllers address that note; channels form Zones, each with a Master Channel for shared messages; when notes outnumber channels they share and lose individual control. MPE v1.1 (M1-100-UM) is an editorial update; a MIDI 2.0 MPE Profile adds two-way negotiation. **[extract]** https://midi.org/midi-polyphonic-expression-mpe, https://midi.org/the-midi-association-announces-the-v1-1-update-to-the-midi-polyphonic-expression-mpe-specification
- Live 11 Max for Live treats MPE data as continuous streams per note. **[extract]** https://www.ableton.com/en/blog/live-11-and-max-for-live-get-a-glimpse-of-whats-possible-with-some-free-devices
- **Confidence:** High. TAP already uses MPE as a carrier for per-note tuning, exactly as the spec intends.

## 6. Browser, terminal and app-based tools

- **Orca** (Hundred Rabbits, Devine Lu Linvega): an "esoteric programming language and livecoding environment for creating procedural sequencers". Every letter is an operator on a grid; it sends MIDI, OSC and UDP and makes no sound itself. C and Electron versions. **[extract]** https://github.com/hundredrabbits/orca-c **Confidence:** Medium.
- **Strudel** (Felix Roos, Alex McLean): a JavaScript port of the TidalCycles pattern language that runs in the browser, outputs WebAudio, MIDI, OSC and more, and works offline as a PWA. **[extract]** https://iclc.toplap.org/2023/catalogue/paper/strudel-live-coding-patterns-on-the-web.html **Confidence:** Medium.
- **Bloom: 10 Worlds** (Brian Eno, Peter Chilvers, Opal, 2018): a reimagining of the 2008 generative app; taps seed an evolving piece; iOS and Android. **[extract]** https://apps.apple.com/us/app/id1438214016, https://enoshop.co.uk/blogs/news/brian-eno-peter-chilvers-release-new-app-bloom-10-worlds **Confidence:** Medium.
- **Leimma and Apotome:** see 1.4.

## 7. Patterns across the field

Each pattern below is **[inference]**, drawn from the sections cited.

1. **History as an input.** TAP, Turing Machine, Marbles, Hermode and Pivotuner all make the current output depend on what came before. The good ones give a way to control how much history counts (Marbles' déjà vu, Turing Machine's slip zone, Hermode's Depth, Pivotuner's key lock).
2. **Bounded drift.** Hermode's zero-sum rule and Pivotuner's pivots keep adaptive systems musical. Unbounded drift (TAP's current modes, bounded only by Lowest and Highest) is more "annoying", which is TAP's point, but an optional anchor would widen its use.
3. **Simulation in place of parameters.** Bastan's devices and Synplant replace knobs with a world (gravity, particles, growth, genes). The user steers the world, not the values.
4. **Small languages.** Permut8's operators, Mozaic scripts, Orca's letters, Strudel patterns and Live 12's MIDI Tools API give users a grammar, which outlives any preset list.
5. **Shared state across the session.** MTS-ESP, Microtuner's Lead/Follow and Live 12 Tuning Systems treat tuning as session state. Most stateful MIDI devices keep their state private.
6. **Learned engines with consent.** RAVE, DDSP and Neutone let users train on their own sound; Combobulator and Neutone Cocoon add revenue to the people whose sound trained the model.
7. **Legibility.** The successful unstable tools show their state: Bastan's visuals, Synplant's branches, Fugue Machine's playheads, Permut8's bit switches. TAP's state (128 drifted pitches) is currently invisible.

## 8. Design ideas and gaps for Max for Live devices like TAP

All **[inference]** unless cited. Target: Live 11, Max 8, `js` (ES5) engine, MPE out.

### 8.1 Ideas that extend TAP directly

1. **Anchor (Hermode-style zero-sum).** After each release, subtract the mean drift of all keys (or of the last N released keys) so the instrument never wanders off on average. A single on/off or 0 to 100% control, like Hermode's Depth (1.1).
2. **Lock keys (Pivotuner-style).** A held key or a key set (for example the tonic and fifth) is excluded from changes, giving fixed pivots inside drift (1.2).
3. **Déjà vu / memory knob (Marbles, Turing Machine).** For Wander: record the random offsets in a loop of N releases; one knob from "always new" to "repeat the loop exactly". For all modes: a probability that a release applies the mode at all (2.2, 2.3).
4. **Visible pitch table (Bastan triad).** A `jsui` or `live.*` display of the 128-key drift as dots or springs. Gravity and Repel become literally visible (2.1).
5. **Seed and branches (Synplant).** "Plant" snapshots the current pitch table; branch slots hold variations made by running a mode with different Mod values; velocity or key range chooses which branch is active (3.1).
6. **Persist the table.** TAP's spec says the pitch table resets when a Set is reopened. Saving it (for example via `pattr` or a hidden `live.*` data store) turns a session's drift into something you can come back to. Most stateful tools in this list save their state.
7. **Publish tuning to the session.** Make TAP an MTS-ESP master, or follow Microtuner's model, so non-MPE synths (and Operator, which TAP cannot reach via MPE) follow TAP's drift. Needs an MTS-ESP external or bridge; `js` cannot load `libMTS` (1.3, 1.5).

### 8.2 New device ideas from the same family

1. **Gravity sequencer for Live 11.** Bouncy Notes needs Live 12; a Live 11 MIDI effect where played notes become bodies that collide, orbit or decay into new notes would fill that gap (2.1).
2. **Multi-playhead MIDI effect.** Record the last bar of input and read it back with up to four playheads at different rates and directions, Fugue Machine style (2.4).
3. **Operator mode (Permut8 / Mozaic / Orca).** A TAP-like device where the "mode" is a two-instruction program (operator plus operand) chosen from a small set (swap, add, pull, rotate, copy), so users invent modes without editing JS (4.1, 2.5, 6).
4. **Adaptive just intonation MIDI effect.** Retune each new note to a pure ratio from a floating centre that follows recent notes (Xenharmonic definition, Sethares, Pivotuner). Shares TAP's MPE output path (1.7).
5. **Neural audio device via `nn~`.** An audio effect loading a RAVE model, if `nn~` loads in Max 8 under Live 11. Would need testing first (3.4).

### 8.3 Gaps and open questions

- **Live 12 lock-in:** MIDI Transformations and Generators, Inspired by Nature (current version) and Live 12 Tuning Systems need Live 12. The repo targets Live 11, so clip-level generative tools and native tuning integration are out of reach without upgrading.
- **No first-hand reads:** every claim here is from search extracts. Before building on any specific number (prices, version numbers, Microtuner's MTS-ESP behaviour, `nn~` Max 8 support), fetch the maker page.
- **Unverified status:** Leimma and Apotome availability, Expressive E's company status, Permut8 and Glitchmachines update status, Magenta Studio maintenance, and the Turing Machine Max for Live port's Live 11 compatibility.
- **Not found:** no tool found does exactly what TAP does (retune a key because it was released, using the previously released key). The nearest are adaptive tuners (which retune by harmonic context, not by release order) and Bouncy Notes (which generates notes, not tunings). That gap is TAP's niche.

## Sources

Adaptive and dynamic tuning
- https://support.apple.com/ar-qa/guide/logicpro/lgcpa88a63e7/mac
- https://help.apple.com/logicpro/mac/10.1/en.lproj/lgcpa88a63e7.html
- https://help.apple.com/logicpro/mac/9.1.6/en/logicpro/usermanual/chapter_43_section_6.html
- https://support.apple.com/guide/logicpro-ipad/lpip67dc8147/ipados
- https://arxiv.org/pdf/2306.03873
- https://www.kvraudio.com/product/pivotuner-by-dmitri-volkov
- https://midi.org/?p=2041
- https://sporkl.gumroad.com/l/hsvkby
- https://cdm.link/inside-mts-esp-the-new-tuning-tool-from-oddsound-and-aphex-twin/
- https://musictech.com/news/aphex-twin-and-oddsound-mts-esp-plug-in-musical-tuning/
- https://www.gearnews.com/oddsound-mts-esp-suite-set-your-tuning-free-at-last
- https://equipboard.com/items/oddsound-mts-esp-suite
- https://github.com/baconpaul/simple-mts-main
- https://marketplace.juce.com/products/juce_mts_esp
- https://crackmagazine.net/2021/03/aphex-twin-collaborates-with-uk-startup-on-microtuning-synth-plug-in/
- https://sevish.com/2021/leimma-apotome-two-new-web-tools-for-microtonal-music/
- https://cdm.link/escaping-tunings-restrictive-default-setting-a-conversation-with-khyam-allami/
- https://redef.com/item/603806c52a569f07a118c57c
- https://www.ableton.com/de/live-manual/12/using-tuning-systems/
- https://www.ableton.com/en/packs/microtuner/
- https://musictech.com/news/gear/ableton-debuts-free-downloadable-midi-device-microtuner
- https://giveitaspin.gr/2021/10/introducing-freedom-of-pitch-infinitone-dmt.html
- https://www.kvraudio.com/marketplace/infinitone-2-by-infinitone
- https://sethares.engr.wisc.edu/html/adaptun.html
- https://arxiv.org/pdf/1706.04338
- https://en.xen.wiki/w/Adaptive_JI

Stateful, generative and physics-based MIDI
- https://www.ableton.com/en/packs/inspired-nature/
- https://www.ableton.com/fr/blog/world-of-sound-explore-live-11s-inspired-by-nature-devices
- https://www.ableton.com/fr/blog/dillon-bastan-water-ripples-and-sound-waves
- https://www.attackmagazine.com/technique/tutorials/getting-freaky-with-live-11s-new-inspired-by-nature-devices/
- https://cdm.link/newswires/tom-whitwell-on-turing-machine-and-how-to-design-your-first-module/
- https://github.com/stellare-modular/vcv-rack
- https://www.synthtopia.com/content/2017/12/01/free-vcv-rack-software-modular-synth-gets-ableton-link-turing-machine-modules/
- https://mindlesstrx.gumroad.com/l/turing
- https://pichenette.readthedocs.io/en/latest/modules/marbles/manual/
- https://www.soundonsound.com/reviews/mutable-instruments-marbles
- https://modulargrid.net/e/before-void-marbles-clone
- https://apps.apple.com/app/id1014191410
- https://apps.apple.com/app/id6670253122
- https://alexandernaut.com/fuguemachine/
- https://www.soundonsound.com/reviews/bram-bos-rozeta-sequencer-suite
- https://cdm.link/mozaic-scriptable-midi-ipad-iphone/
- https://apps.apple.com/us/app/mozaic-plugin-workshop/id1457962653
- https://ableton.com/en/live/all-new-features
- https://www.soundonsound.com/techniques/ableton-live-12-midi-generators
- https://musicradar.com/how-to/live-12-midi-tools
- https://docs.cycling74.com/userguide/m4l/live_miditools/
- https://docs.cycling74.com/reference/live.miditool.out
- https://www.ableton.com/en/packs/midi-tools

Evolutionary and machine-learning instruments
- https://cdm.link/synplant-2-hands-on/
- https://synthanatomy.com/2023/10/sonic-charge-synplant-2-popular-synth-plugin-gets-a-genetic-sound-design-boost.html
- https://www.soundonsound.com/reviews/sonic-charge-synplant-2
- https://musictech.com/reviews/plug-ins/sonic-charges-synplant-2-review/
- https://magenta.tensorflow.org/studio-announce
- https://www.ableton.com/en/blog/magenta-studio-free-ai-tools-ableton-live
- https://github.com/tensorflow/magenta-studio
- https://qosmo.notion.site/Getting-started-607e99436b1243b5a6d273a76ee4811a
- https://qosmo.jp/en/news/neutone
- https://qosmo.jp/en/news/neutone-project-lydia
- https://sonicstate.com/news/2024/04/26/neutone-morpho-v1-released/
- https://neutone.supabase.co/storage/v1/object/public/morpho-public/manual/2026-06-25_NeutoneMorpho_Manual.pdf
- https://audiocipher.com/post/neutone
- https://webdev.apsipa.org/proceedings/2025/papers/APSIPA2025_P071.pdf
- https://github.com/acids-ircam/RAVE
- https://pypi.org/project/acids-rave
- https://magenta.tensorflow.org/ddsp-vst
- https://magenta.tensorflow.org/ddsp-vst-blog
- https://datamindaudio.ai/wp-content/uploads/2024/01/Combobulator-User-Manual-v0.9.2.pdf
- https://www.musicradar.com/news/datamind-audio-combobulator
- https://www.native-instruments.com/en/products/nks-partners/datamind-audio/combobulator/
- https://www.recordoftheday.com/news-and-press/datamind-audio-launches-long-awaited-virtual-ai-instrument-and-ethically-sourced-artist-brains-for-windows-users

Destructive and unstable effects
- https://cdn.soniccharge.com/permut8
- https://www.soundonsound.com/node/4906681
- https://www.attackmagazine.com/reviews/gear-software/sonic-charge-permut8/
- https://www.plugin-alliance.com/products/byome
- https://www.sweetwater.com/store/detail/SpecOps--unfiltered-audio-specops
- https://www.fast-and-wide.com/equipment-releases/outboard/10270-unfiltered-audio-specops
- https://sonicstate.com/news/2014/02/03/free-plug-in-creates-robotic-artifacts-/
- https://www.synthtopia.com/content/2014/06/30/free-glitch-delay-plugin-for-mac-windows-glitchmachines-hysteresis/
- https://synthanatomy.com/?p=85427
- https://www.native-instruments.com/en/products/komplete/synths/flesh/
- https://cdm.link/flesh-from-tim-exile-transforms-sounds-into-performances/

Novel synthesis and expressive interfaces
- https://cdm.link/2024/05/madrona-labs-sumu-hands-on/
- https://rekkerd.org/madrona-labs-launches-sumu-software-synthesizer
- https://kvraudio.com/news/madrona-labs-updates-its-synthesizer-lineup-46868
- https://synthtopia.com/content/2015/07/30/aalto-1-7-brings-support-for-multidimensional-polyphonic-expression
- https://www.sweetwater.com/store/detail/Osmose
- https://kraftmusic.com/products/expressive-e-osmose-61-key-synthesizer-mpe-midi-controller
- https://roli.com/blog/roli-reforms-as-luminary-what-you-need-to-know
- https://cdm.link/roli-hits-reboot-files-for-administration-as-it-shifts-to-beginners-and-subscriptions/
- https://midi.org/midi-polyphonic-expression-mpe
- https://midi.org/the-midi-association-announces-the-v1-1-update-to-the-midi-polyphonic-expression-mpe-specification
- https://www.ableton.com/en/blog/live-11-and-max-for-live-get-a-glimpse-of-whats-possible-with-some-free-devices

Browser, terminal and app-based tools
- https://github.com/hundredrabbits/orca-c
- https://iclc.toplap.org/2023/catalogue/paper/strudel-live-coding-patterns-on-the-web.html
- https://apps.apple.com/us/app/id1438214016
- https://enoshop.co.uk/blogs/news/brian-eno-peter-chilvers-release-new-app-bloom-10-worlds
