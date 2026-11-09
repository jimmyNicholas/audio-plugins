# Audio plugins

Max for Live devices for Ableton Live 11 and Max 8.

## Devices

| Device | What it does | Docs |
| --- | --- | --- |
| [TAP](devices/tap/) | The Annoying Piano: a MIDI effect that retunes each key as you play and sends per-note pitch (MPE) to the instrument after it. | [Spec](devices/tap/docs/SPEC.md), [handoff](devices/tap/docs/HANDOFF.md), [tickets](devices/tap/docs/tickets/) |
| [Slush Wave](devices/slush-wave/) | Paused. A planned family of audio effects for the slushwave sound; so far research and a pitch layer audition prototype. | [Handoff](devices/slush-wave/HANDOFF.md), [context](devices/slush-wave/CONTEXT.md) |

TAP started as part of the web app [next-annoying-piano](https://github.com/jimmyNicholas/next-annoying-piano) and moved here with its history on 2026-10-28.

## Layout

```
devices/<name>/      one folder per device: patch, scripts, tests, docs
tools/amxd.mjs       packs a .maxpat into an .amxd, and back; shared by every device
```

Code moves into a shared folder once a second device needs it, not before.

## Using a device

1. In Live's browser, Places > Add Folder, and choose the device's folder in your checkout.
2. Drag the `.amxd` onto a MIDI track, before an MPE instrument such as Wavetable.

## Tests

`npm test` from the repo root runs every test. No dependencies; Node 18 or later is enough. CI also checks that device scripts are ES5, since that is all Max 8's `js` object understands.

## Business plan

[`docs/business/plugin-business-plan.md`](docs/business/plugin-business-plan.md): how the plugins could make money, with the decisions made so far, the paths and trade-offs still open, costs, and the planned release pipeline.

## Research

Research that is not tied to one device lives in [`docs/research/`](docs/research/):

- [Boundary-pushing plugins](docs/research/boundary-pushing-plugins.md): audio and MIDI tools that change how music is made, and gaps for devices like TAP.
- [Boundary-pushing music philosophy, 2016 to 2026](docs/research/boundary-pushing-music-philosophy.md): ideas from the last ten years, turned into design prompts.
- [How small plugin and Max for Live developers make money](docs/research/indie-plugin-monetisation.md): 26 real cases sorted into common, interesting and unique patterns, applied to this project, with three costed paths and the changes they suggest to the [business plan](docs/business/plugin-business-plan.md).
- [The Bandcamp discography model](docs/research/bandcamp-discography-model.md): selling many small, cheap items singly or as a cheap "everything so far" bundle, from Geometric Lullaby and signalwave to sample pack sellers, with what Bandcamp allows (audio, not plugins) and how to combine it with itch.
- [Recent ambient, dark ambient and slushwave practice](docs/research/recent-ambient-practice.md): pitched-down and AI-free generative moves since 2016, with device ideas.
