# Handoff: Slush Wave

Last updated 2026-11-01. **Paused** by the user "until I have a better idea of what I would like". Work was on the branch `claude/slush-wave-plugin-wayfinding-kyev7g`, which the user is merging into `main` at the pause.

## Goal

A family of Max for Live devices plus an Audio Effect Rack preset that give a track the slushwave sound. Terms are in [`CONTEXT.md`](CONTEXT.md). The plan lives on GitHub as a map of decision tickets: [#1 Map: Slush Wave](https://github.com/jimmyNicholas/audio-plugins/issues/1). Research notes are in [`docs/research/`](docs/research/).

## Where the map stands

| Ticket | State |
| --- | --- |
| #2 to #6, #9 (research and grilling) | **Closed.** Answers are in `docs/research/` and in the tickets. |
| #4 amxd tool for audio effects | **Done.** `tools/amxd.mjs pack --type audio`, with a check that the patcher's `project.amxdtype` matches. |
| #7 What controls does the player get? | Open. Waits on #10. |
| #8 Build and check the Slush Wave spike in Live | Open. Not started. |
| #10 How is Slush Pitch Layer built? | Open. The audition (#14) exists to answer it. |
| #11 How is Slush Phaser built? | Open. Not started. |
| #12 Is Chorus-Ensemble enough for wobble? | Open. Not started. |
| #14 Build and hear the Slush Pitch Layer audition | **Built, round 1 tested, round 2 not run.** Details below. Sub-tickets #15 to #19 are built. |

## The Slush Pitch Layer audition (#14)

Folder: [`prototype/pitch-layer-audition/`](prototype/pitch-layer-audition/). A throwaway A/B rig for hearing the candidate ways to build the pitch layer (detuned doubles plus an octave or fifth down) in Live 11. It is never released.

- [`SPEC.md`](prototype/pitch-layer-audition/SPEC.md): the build spec, decisions Q1 to Q8 and the stages.
- [`README.md`](prototype/pitch-layer-audition/README.md): the user's walkthrough, a pass/fail table per stage. Blank and ready for round 2.
- [`RESULTS.md`](prototype/pitch-layer-audition/RESULTS.md): round 1 answers and what they showed.
- `build-patch.mjs`: generates every `.maxpat` and packs every `.amxd` (`npm run build` in that folder). Edit this, never the patches.
- `audition.test.mjs`: structural checks per device (part of `npm test`).

**Seven devices**, one question each, all built from the same generator pieces:

| Stage | Device | Tests |
| --- | --- | --- |
| 1 | Audition 1 Alignment | The dry path alone, delayed by a fixed 8192 samples (the Defined Latency) |
| 2 | Audition 2 Doubles Delay Line | Steady delay line doubles and the Vibrato doubles, Source |
| 3 | Audition 3 Doubles Pitchshift | `pitchshift~` doubles, Quality, measured latency |
| 4 | Audition 4 Doubles Drift | Slow random wander on the doubles |
| 5 | Audition 5 Interval Pitchshift | Octave or fifth down with `pitchshift~`, Quality, Stereo |
| 6 | Audition 6 Interval Delay Line | The same with the delay line shifter, Window |
| 7 | Audition 7 Combined | Everything together, CPU, favourite settings |

**How it works, in short:** the dry signal is delayed by exactly 8192 samples. Each engine is its own subpatcher behind a `mute~`, so only the selected one uses CPU. Each `pitchshift~` measures its own latency and delays itself by 8192 minus that, so everything lines up. Controls reach the subpatchers by `---` send and receive names.

### Round 1 (2026-11-01): what we learned

- **Works:** alignment (stage 1); Solo, Layers and switching; Vibrato; the `pitchshift~` doubles (+/-12 cents on a tuner); the `pitchshift~` interval (-5 cents on the octave, -2 on the fifth).
- **Was broken, now fixed, not yet re-tested:** the delay line shifter. It used `expr~`, which is Pure Data, not Max (`newobj: expr~: No such object`). That is why the Delay line engine never pitched in stages 2, 4 and 6. It now uses `expr` at control rate. A test now only allows objects on a list of known Max 8 objects.
- **Unclear:** Quality. The latency read 4096 samples (92.9 ms) at every Quality and no difference was heard. Either `pitchshift~` really has the same latency at every quality, or the `quality` message isn't taking effect. There was no console error.
- **Not really tested:** Source and Stereo made no difference, probably because the loop wasn't wide. Round 2's README says to put Auto Pan before the device.
- **CPU is not a concern:** 6 to 7% at a 512-sample buffer, and 20 tracks with the device used about 15%.
- No preferences yet: the user was unsure which engines or settings they liked.

### To pick it up again

1. Pull `main`, run `npm test` (should be all passing).
2. Round 2 in Live: stages 2, 4 and 6 in full, rows 2.7, 3.3, 3.4 and 5.7. The README marks them.
3. If Quality still makes no difference, add a check that shows whether `pitchshift~` accepted the `quality` setting (for example print its attributes with `getattributes`, or watch CPU change between basic and best).
4. Copy the answers into `RESULTS.md` as round 2 and post a summary on #14. Its answers then settle #10, which unblocks #7 (controls).

### Things to know before changing the audition

- Nobody can run Max in the cloud sessions. Every object name must be one Max 8 really has; the Cycling '74 docs site was blocked there, so `pitchshift~`'s attribute and message names (`pitchshift`, `quality`, `reportlatency`, `getlatency`, the `latency <n>` reply) come from search extracts. The README's troubleshooting section lists them.
- The Max Console is the fastest way to find a fault: ask for its red lines first.
- Hard rules from the spec: no `gen~` (the licence can't edit it), no `js`, no third-party packages, unfrozen devices, every subpatcher embedded, Australian spelling and no em dashes.
- The Vibrato engine came from a reconstruction made in another chat (a zip of seven devices, not committed). Only that idea was adopted; the rest of that zip disagreed with the spec (no `pitchshift~`, manual alignment).
