# Audio plugins

Max for Live devices for Ableton Live 11 and Max 8.

## Devices

| Device | What it does | Docs |
| --- | --- | --- |
| [TAP](devices/tap/) | The Annoying Piano: a MIDI effect that retunes each key as you play and sends per-note pitch (MPE) to the instrument after it. | [Spec](devices/tap/docs/SPEC.md), [handoff](devices/tap/docs/HANDOFF.md), [tickets](devices/tap/docs/tickets/) |
| [Band Spread](devices/band-spread/) | Planned. An audio effect that cuts one narrow band and plays copies of it at other places in the spectrum. | [Context](devices/band-spread/CONTEXT.md), [map](https://github.com/jimmyNicholas/audio-plugins/issues?q=label%3Awayfinder%3Amap+%22Band+Spread%22) |
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
