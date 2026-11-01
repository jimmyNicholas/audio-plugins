# Slush Pitch Layer audition: how to test it

Seven small Max for Live audio effects. Stages 1 to 6 each test one piece of the Slush Pitch Layer on its own, and stage 7 puts them all together for the final comparisons. Your notes settle [How is Slush Pitch Layer built?](https://github.com/jimmyNicholas/audio-plugins/issues/10). Post the results on [#14](https://github.com/jimmyNicholas/audio-plugins/issues/14).

**Round 2:** re-run stages **2, 4 and 6** (the Delay line engine was broken in round 1 and is fixed), plus **3.4** and the **Source** and **Stereo** rows with a properly wide loop. Round 1's answers are in [RESULTS.md](./RESULTS.md).

Work through them in order. Each stage takes 5 to 10 minutes. If a stage fails, stop there and send back what you have: the later stages build on it.

None of these are released. The build spec is [SPEC.md](./SPEC.md).

## Getting started (once)

1. Switch to the branch `claude/slush-wave-plugin-wayfinding-kyev7g` and pull.
2. Optional: in a terminal, from the repo root, run `npm test`. The last lines should show `# fail 0`.
3. In Live 11's browser, under **Places**, click **Add Folder…** and choose `devices/slush-wave/prototype` inside your `audio-plugins` checkout.
4. In Live, check **Options > Delay Compensation** is ticked.
5. Use 44.1 or 48 kHz.

### Test material

| Name | How to make it | Good for |
|---|---|---|
| **Loop** | An audio track with a loop, Warp mode **Re-Pitch**, the set's tempo at about 0.8 times the loop's own (for a 100 BPM loop, 80 BPM) | Listening, timing |
| **Sine** | A MIDI track with **Operator** (its default sound is a sine). Hold **A4** (440 Hz), or draw a long A4 in a clip | Pitch checks with a tuner |
| **Wide loop** | The **Loop** track with Live's **Auto Pan** added *before* the audition device: **Amount 100%**, **Rate** about 0.5 Hz. The loop now swings between left and right, so the two sides differ | Source and Stereo checks |

### The tuner chain (for pitch checks)

After the audition device on the **Sine** track, add **Utility** and then **Tuner**. In Utility, set **Channel** (the mode menu) to **Left** to read the left side, or **Right** to read the right side. Tuner then shows the note and how many cents sharp or flat it is.

### Every device has

- **Defined Latency: 8192 smp** on the left. The dry signal is delayed by exactly that, and Live makes up for it, so the track stays in time with the rest of the set.
- From stage 2 on: **Layers** (Off gives the plain, delayed input), **Solo** (Off, or the layer on its own without the dry signal), and the layer's **Level**. Level at its minimum (-70 dB) switches the layer off.
- When there are two engines, the old one fades out at once and the new one fades in about a quarter of a second later, so leftover sound in its buffers is never heard. Judge the sound a moment after a switch.
- **Trim** dials (one per engine) to match the engines' loudness before you compare them.

If a device shows an error or makes no sound, see "Troubleshooting" at the end.

---

## Stage 1: **Audition 1 Alignment.amxd**

Just the dry path: the input, delayed by 8192 samples. Checks that Live lines it up, which every later stage relies on.

**Setup:** the **Loop** track with the device on it. Duplicate the track (**Cmd+D**), delete the device from the copy, and add **Utility** to the copy with both **Phz-L** and **Phz-R** on (phase invert). Both tracks at the same volume.

| # | Do | Expect | Pass |
|---|---|---|---|
| 1.1 | Play both tracks | Near silence: the copy cancels the original | [ ] |
| 1.2 | Turn the device off (round button at the left of its title bar), then on | Near silence either way | [ ] |
| 1.3 | Turn Utility's phase buttons off on the copy | The loop, louder, with no flam or echo | [ ] |
| 1.4 | Remove the device and Utility from the tracks, or mute the copy | Done | [ ] |

If 1.1 or 1.2 is not silent, stop and tell me what you hear (a thin, phasey sound means the timing is off).

## Stage 2: **Audition 2 Doubles Delay Line.amxd**

The detuned doubles from a delay line, in two flavours. **Delay line** holds a steady detune (up on the left, down on the right). **Vibrato** is the alternative from the other chat: a sine LFO sweeps a short delay, so the pitch wobbles either side, up to Cents at its peak, at **Vib Rate**. Answers Q1 (in part) and Q5.

| # | Do | Expect | Pass |
|---|---|---|---|
| 2.1 | **Sine** with the tuner chain. **Solo Doubles**, **Engine Delay line**, **Cents 12**. Utility on Left | Tuner reads A, about **+12** cents, steady (it may flicker a little) | [ ] |
| 2.2 | Utility on Right | About **-12** cents | [ ] |
| 2.3 | **Engine Vibrato**, **Vib Rate 0.67**, Utility on Left | The reading swings between about -12 and +12, about two-thirds of a cycle a second | [ ] |
| 2.4 | **Loop**, **Solo Doubles**. Flip **Engine** a few times; match loudness with **Trim DL** and **Trim VB** | Both work. Note which you prefer, and the trims | [ ] |
| 2.5 | Try **Cents 5** and **Cents 25** on both engines | Note how each sounds | [ ] |
| 2.6 | **Solo Off** (doubles under the dry loop) | Thicker and wider. Note any hollow or phasey tone | [ ] |
| 2.7 | **Wide loop**, **Solo Doubles**. Flip **Source** between **Per channel** and **Mono sum** | Per channel: the doubles swing left and right with the loop. Mono sum: they stay put, one on each side. Then **Solo Off** and note which you prefer | [ ] |
| 2.8 | **Layers Off**, then **On** | Off gives the plain loop, no pop | [ ] |

**Report:** pass or fail for each, which engine you prefer and at what Cents (and Vib Rate for Vibrato), and Per channel or Mono sum.

## Stage 3: **Audition 3 Doubles Pitchshift.amxd**

The same steady doubles, made with Max's `pitchshift~` instead. Adds the **Quality** menu and a latency readout. Answers Q1 (the other half) and Q8.

| # | Do | Expect | Pass |
|---|---|---|---|
| 3.1 | Load it and play anything | Within a second, **Doubles PS** on the left changes from "not measured yet" to a number of samples and ms. Write it down | [ ] |
| 3.2 | **Sine** with the tuner chain, **Solo Doubles**, **Cents 12**, Utility on Left, then Right | About **+12** cents, then about **-12**, both steady | [ ] |
| 3.3 | Step **Quality** through basic, good, better, best, waiting a second each time | Write down the number each time. In round 1 it stayed at 4096; if it still does, say whether any Quality sounds different | [ ] |
| 3.4 | **Loop** (one with clear drum hits), **Solo Off**, Quality **good**. Listen to the hits | Each hit sounds like one hit. Fail if you hear each hit twice, close together (a flam or short echo) | [ ] |
| 3.5 | Put stage 2's device on a copy of the track and A/B with this one | Note which you prefer, and whether you can tell them apart under the dry loop | [ ] |

**Report:** the readout for each Quality, the tuner readings, and your preference against stage 2.

## Stage 4: **Audition 4 Doubles Drift.amxd**

Slow random wander on the doubles' pitch, for both steady engines. Each double wanders on its own. Answers Q7.

| # | Do | Expect | Pass |
|---|---|---|---|
| 4.1 | **Sine** with the tuner chain, **Solo Doubles**, **Engine Delay line**, **Cents 12**, **Drift On**, **Depth 3**, **Rate 0.2**, Utility on Left | The reading wanders slowly around +12, by up to about 3 cents either way | [ ] |
| 4.2 | Utility on Right | Wanders around -12, not in step with the left | [ ] |
| 4.3 | **Engine Pitchshift** | Same, but in small steps (it updates every 20 ms) | [ ] |
| 4.4 | **Drift Off** | Steady again on both engines | [ ] |
| 4.5 | **Loop**, **Solo Off**. Try Drift with Depth 1.5 to 6 and Rate 0.1 to 0.5 on both engines | Note whether it helps, and the settings you liked | [ ] |

**Report:** Drift Off or On, the settings you liked, and on which engine.

## Stage 5: **Audition 5 Interval Pitchshift.amxd**

The interval layer (an octave or a fifth down) with `pitchshift~`. Answers Q2 (quality and latency) and Q6.

| # | Do | Expect | Pass |
|---|---|---|---|
| 5.1 | Load it and play anything | **Interval PS** on the left shows a number. Write it down | [ ] |
| 5.2 | **Sine** with the tuner chain, **Solo Interval**, **Octave down** | Tuner reads **A3** (220 Hz), close to 0 cents | [ ] |
| 5.3 | **Fifth down** | Tuner reads **D4** (293.7 Hz), close to 0 cents | [ ] |
| 5.4 | **Loop**, **Solo Interval**, **Octave down**. Step **Quality** through basic to best | The readout updates each time. Write down each number and how each sounds | [ ] |
| 5.5 | **Solo Off**, **Level -12**. Judge each Quality under the dry loop | Note the lowest Quality that sounds fine | [ ] |
| 5.6 | Repeat 5.4 and 5.5 with **Fifth down** | As above | [ ] |
| 5.7 | **Wide loop**, **Solo Interval**. Flip **Stereo** between **Mono centre** and **Per channel** | Mono centre: the interval sits still in the middle. Per channel: it swings with the loop. Then **Solo Off** and note which you prefer | [ ] |

**Report:** the readout per Quality, the lowest Quality that's fine (octave and fifth), and Mono centre or Per channel.

## Stage 6: **Audition 6 Interval Delay Line.amxd**

The interval layer from the delay line shifter: cheaper, with a grain you may or may not want. Answers Q2 (engine).

| # | Do | Expect | Pass |
|---|---|---|---|
| 6.1 | **Sine** with the tuner chain, **Solo Interval**, **Octave down**, then **Fifth down** | **A3**, then **D4**, as in stage 5. The reading may flicker more | [ ] |
| 6.2 | **Loop**, **Solo Interval**. Try **Window** 30, 60 and 120 | Shorter is tighter but buzzier, longer is smoother but more smeared | [ ] |
| 6.3 | **Solo Off**, **Level -12**, under the dry loop | Note whether the grain is usable, or even wanted | [ ] |
| 6.4 | A/B with stage 5 on a copy of the track | Note which you prefer | [ ] |

**Report:** the Window you liked, and this against stage 5.

## Stage 7: **Audition 7 Combined.amxd**

Everything together, with the same controls as the stages before. Use it for the final balance, the CPU check and your favourite settings.

| # | Do | Expect | Pass |
|---|---|---|---|
| 7.1 | **Loop**. **Solo Doubles**, then **Solo Interval**, then **Solo Off**, then **Layers Off** | Only the doubles, only the interval, everything, then only the plain loop | [ ] |
| 7.2 | **Solo Off**. Set your favourites from stages 2 to 6. Flip **D Engine** and **I Engine** under the full sound | Note whether your favourites still win | [ ] |
| 7.3 | Note your buffer size (**Live > Preferences > Audio**). Read Live's CPU meter (top right) with the device off, then on with each **D Engine** and each **I Engine**, and with the interval on Pitchshift at each **Quality** | Write down the readings | [ ] |
| 7.4 | Optional: duplicate the track (with the device) until the CPU meter reaches about 50%, with your favourite settings | Write down how many copies | [ ] |
| 7.5 | Set up your favourite combination | Write down every setting on the panel | [ ] |

## What to send back

- The ticked tables (pass or fail for each row) and the reports for each stage.
- The latency readouts: doubles and interval, at each Quality.
- The CPU readings and your buffer size.
- Your favourite combination from 7.5.
- Your Live version (**Live > About Live**) and Max version (top of the Max Console, or **Max > About Max** in the Max editor).
- Any red lines from the Max Console.

## Troubleshooting

**If a device shows an error or makes no sound:** click the **Edit** button in its title bar (the small icon at top right) to open the Max editor, open **Window > Max Console**, and copy every red line into your reply. If Live refuses to load a device at all, copy the message.

The builder could not run Max, and the `pitchshift~` reference page was not reachable, so a few names were taken from search extracts. These are the first places to look.

- **`pitchshift~` attribute or message names** (stages 3, 4, 5, 7). The patch uses the `pitchshift` attribute (a pitch factor), `quality` (`basic`, `good`, `better`, `best`), `reportlatency 1`, and the `getlatency` message. It expects the latency as `latency <samples>` from the right-hand outlet. A red line like `pitchshift~: doesn't understand ...` names the one that is wrong.
- **A latency readout stays at "not measured yet".** `pitchshift~` may only report its latency while audio is running. Play something, then change **Quality** and back, or switch the **Engine** away and back; either measures again. If it still doesn't change, the latency message isn't reaching the patch.
- **The Quality menu** affects every `pitchshift~` in the device, doubles and interval alike.
- **Readouts show odd text.** They are built with `sprintf`. Copy what the panel shows.
- **A warning line appears.** A measured latency is over 8192 samples, so that version can't be lined up with the dry signal. The Max Console has a matching `audition: warning ...` line. Report the number.
- **`mute~`.** Each engine sits behind a `mute~`. If an engine you're not using still costs CPU, say so.
- **CPU and Stereo.** The interval always runs two shifters, even in Mono centre (the right one is just not heard), so Stereo doesn't change the CPU reading.
- **Clicks.** Switching Source, Stereo or Interval can click; that's expected.
- **Utility's Channel menu** (for the tuner chain). If your Utility has no Left or Right mode, say so and skip the tuner rows; I'll add a channel switch to the devices.

The Max Console lines from the devices start with `audition:`, for example `audition: pitchshift interval good latency <n> samples`, with the number in place of `<n>`.

## Notes

- Don't save changes to a device from the Max editor. The repo copies are generated by `build-patch.mjs`; `npm run build` in this folder rewrites every `.maxpat` and `.amxd`.
