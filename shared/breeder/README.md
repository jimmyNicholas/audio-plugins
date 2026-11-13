# Shared: breeder devices

Code shared by the breeder devices ([Kick Breeder](../../devices/kick-breeder/), [Snare Breeder](../../devices/snare-breeder/)). The choosing loop, the panel, the keys and saving are the same for every drum, so they live here. Each device has only its sound: a voice file and the MSP objects in its `build-patch.mjs`.

| File | What it is |
| --- | --- |
| `dsp.js` | Maths every voice uses: setting ranges, envelopes as `line~` breakpoints, envelope compression, and the pieces of the loudness simulation (noise, a filter, weighting, matching). ES5. |
| `engine.js` | `makeBreeder(drum)`: the breeding engine (rounds, variations, locks, Back, Reset, the seeded random numbers). Pure. ES5. |
| `glue.js` | The Max glue: panel buttons, keys, saved drums, filling each slot's message boxes. ES5. |
| `patch.mjs` | Builds Max patcher JSON, and knows the inlets and outlets of every object used. |
| `build.mjs` | `breederDevice(config, dir)`: the whole device patch around a voice section, and the joined js. |
| `tests/` | `dsp.test.js` tests the maths. `patch-checks.mjs` is the set of patch checks every device's `tests/patch.test.mjs` runs. |

## How a device's js is made

Max 8's `js` object loads one file, which must sit next to the `.amxd`. So each device's `npm run build` joins:

1. `dsp.js`
2. `engine.js`
3. the device's voice file, which defines `var Drum = { NOUN, PARAMS, PARTS, voice, loudness, describe, label }`
4. `var Engine = makeBreeder(Drum);` and the Node export
5. `glue.js`

into the device's own js (for example `kick-breeder.js`). Never edit that file: edit these, or the voice file, and run `npm run build` in each device folder. The tests fail if a device's js, `.maxpat` or `.amxd` is out of date.

## Adding a drum

1. Copy a device folder. Rename the voice file and write its `PARAMS`, `PARTS` (`"statics"` first, then one name per `line~` envelope), `voice`, `loudness`, `describe` and `label`.
2. In `build-patch.mjs`, write the voice section: the MSP objects, and which box each part feeds, which box resets the oscillator phase, and which takes velocity.
3. `npm run build`, then `npm test` from the repo root.

The breeding is tested through the kick spike (`devices/kick-breeder/spike/tests/engine.test.js`).
