# Audio plugins

Max for Live devices for Ableton Live 11 and Max 8, and JUCE plugins (VST3 and AU).

## Devices

| Device | What it does | Docs |
| --- | --- | --- |
| [TAP](devices/tap/) | The Annoying Piano: a MIDI effect that retunes each key as you play and sends per-note pitch (MPE) to the instrument after it. | [Spec](devices/tap/docs/SPEC.md), [handoff](devices/tap/docs/HANDOFF.md), [tickets](devices/tap/docs/tickets/) |
| [Kick Breeder](devices/kick-breeder/) | Spike. A kick drum instrument with no samples: choose between candidate kicks round after round, and new ones are bred from your choice until you keep one. | [Spec](devices/kick-breeder/docs/SPEC.md), [spike](devices/kick-breeder/spike/README.md) |
| [Snare Breeder](devices/snare-breeder/) | Spike. The snare version of Kick Breeder: skin, snare wires and claps, bred the same way, on the same shared code. | [Spec](devices/snare-breeder/docs/SPEC.md), [spike](devices/snare-breeder/spike/README.md) |
| [Slush Wave](devices/slush-wave/) | Paused. A planned family of audio effects for the slushwave sound; so far research and a pitch layer audition prototype. | [Handoff](devices/slush-wave/HANDOFF.md), [context](devices/slush-wave/CONTEXT.md) |

## Plugins

JUCE plugins, built with CMake. Each builds and tests on its own; see its README.

| Plugin | What it does | Docs |
| --- | --- | --- |
| [Bloodline](plugins/bloodline/) | Spike. A sample-free drum instrument for Bloodline Breaks: breed drums on a workbench by choosing between pads, then place them in a kit of three slots, all from an MPD226. One shared 19-setting voice, with Kick, Snare and Hats as optional zones. | [README and checklist](plugins/bloodline/README.md) |
| [Synth Breeder](plugins/synth-breeder/) | Spike, mouse led. A guess at a synth version: a litter of eight patches to click through while you play or a riff loops, bred, crossed, and steered by zones. | [README](plugins/synth-breeder/README.md) |
| [FX Breeder](plugins/fx-breeder/) | Spike, mouse led. An audio effect that breeds whole chains of up to six effects (which, in what order, and their settings), for one drum or the bus, with a Wild switch. | [README](plugins/fx-breeder/README.md) |

TAP started as part of the web app [next-annoying-piano](https://github.com/jimmyNicholas/next-annoying-piano) and moved here with its history on 2026-10-28.

## Layout

```
albums/NN-<slug>/    one folder per album: manifest, story, art, music, samples
devices/<name>/      one folder per Max for Live device: patch, scripts, tests, docs
plugins/<slug>/      one folder per JUCE plugin: a plain C++ core, the JUCE layer, tests
plugins/shared/      the litter breeding loop and test harness shared by Synth Breeder and FX Breeder
shared/breeder/      the breeding loop, panel and keys shared by Kick and Snare Breeder
tools/amxd.mjs       packs a .maxpat into an .amxd, and back; shared by every device
```

Code moves into a shared folder once a second device needs it, not before. A device still loads one js file next to its `.amxd`, so a device that uses `shared/` joins it into that file when it builds; see [`shared/breeder/`](shared/breeder/README.md).

## Using a device

1. In Live's browser, Places > Add Folder, and choose the device's folder in your checkout.
2. Drag the `.amxd` onto a MIDI track, before an MPE instrument such as Wavetable.

## Tests

`npm test` from the repo root runs every Max for Live and tools test. No dependencies; Node 18 or later is enough. CI also checks that device and shared scripts are ES5, since that is all Max 8's `js` object understands. The JUCE plugins have their own C++ tests, run with `ctest`; see each plugin's README.

## Albums

| Album | Status | Folder |
| --- | --- | --- |
| 1. Bloodline Breaks | Draft: collection stage. Sample-free hip hop drums, bred from nothing. | [`albums/01-bloodline-breaks/`](albums/01-bloodline-breaks/) |

Each album folder is a copy of [`business_plan/templates/albums/_template/`](business_plan/templates/albums/_template/). JUCE plugins live in `plugins/<slug>/` at the root; the first is the Bloodline Breaks spike, [`plugins/bloodline/`](plugins/bloodline/).

## Business plan

[`business_plan/`](business_plan/): how the plugins could make money. Releases as themed albums (plugins, music, sample packs) on my own site, with singles, albums and a discography, and the story as the marketing. Holds the current plan, every decision and why, a summary of how it came about, album templates, and the research behind it.

## Research

Research that is not tied to one device lives in [`docs/research/`](docs/research/):

- [Boundary-pushing plugins](docs/research/boundary-pushing-plugins.md): audio and MIDI tools that change how music is made, and gaps for devices like TAP.
- [Boundary-pushing music philosophy, 2016 to 2026](docs/research/boundary-pushing-music-philosophy.md): ideas from the last ten years, turned into design prompts.
- [Recent ambient, dark ambient and slushwave practice](docs/research/recent-ambient-practice.md): pitched-down and AI-free generative moves since 2016, with device ideas.
- [Composing from nothing, without samples](docs/research/sample-free-composition-tools.md): synthesised drums, free and boundary-pushing synths, and MIDI generators for a fresh start with no sample library.
