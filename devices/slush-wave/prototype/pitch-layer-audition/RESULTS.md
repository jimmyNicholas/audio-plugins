# Slush Pitch Layer audition: results

## Round 1 (2026-11-01)

Your answers from the first pass through the seven devices, copied from the README. Live version, Max version and sample rate weren't given.

**What it found:**

- The delay line shifter didn't load: Max reported `newobj: expr~: No such object` (`expr~` is a Pure Data object, not a Max one). That is why the Delay line engine never pitched in stages 2, 4 and 6. Fixed in round 2 by working out the sweep rate with `expr` at control rate.
- Vibrato, the `pitchshift~` doubles and the `pitchshift~` interval all pitch correctly. The interval read -5 cents for the octave and -2 cents for the fifth.
- Alignment (stage 1) passes.
- The `pitchshift~` latency read 4096 samples (92.9 ms) at every Quality, for both doubles and interval, and no difference between Qualities could be heard. There was no Max Console error for `quality`.
- CPU: 6 to 7% at a 512-sample buffer, and 20 tracks with the device used about 15%.
- Source and Stereo made no audible difference, probably because the loop wasn't wide; round 2's README explains how to make one.

### Stage 1: **Audition 1 Alignment.amxd**

| #   | Do                                                                       | Expect                                      | Pass  |
| --- | ------------------------------------------------------------------------ | ------------------------------------------- | ----- |
| 1.1 | Play both tracks                                                         | Near silence: the copy cancels the original | [ y ] |
| 1.2 | Turn the device off (round button at the left of its title bar), then on | Near silence either way                     | [ y ] |
| 1.3 | Turn Utility's phase buttons off on the copy                             | The loop, louder, with no flam or echo      | [ y ] |
| 1.4 | Remove the device and Utility from the tracks, or mute the copy          | Done                                        | [ y ] |

### Stage 2: **Audition 2 Doubles Delay Line.amxd**

| #   | Do                                                                                                       | Expect                                                                                            | Pass                          |
| --- | -------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- | ----------------------------- |
| 2.1 | **Sine** with the tuner chain. **Solo Doubles**, **Engine Delay line**, **Cents 12**. Utility on Left    | Tuner reads A, about **+12** cents, steady (it may flicker a little)                              | [ no ]                        |
| 2.2 | Utility on Right                                                                                         | About **-12** cents                                                                               | [ no ]                        |
| 2.3 | **Engine Vibrato**, **Vib Rate 0.67**, Utility on Left                                                   | The reading swings between about -12 and +12, about two-thirds of a cycle a second                | [y ]                          |
| 2.4 | **Loop**, **Solo Doubles**. Flip **Engine** a few times; match loudness with **Trim DL** and **Trim VB** | Both work. Note which you prefer, and the trims                                                   | [y ]                          |
| 2.5 | Try **Cents 5** and **Cents 25** on both engines                                                         | Note how each sounds                                                                              | [only vibrato works ]         |
| 2.6 | **Solo Off** (doubles under the dry loop)                                                                | Thicker and wider. Note any hollow or phasey tone                                                 | [ y ]                         |
| 2.7 | **Wide loop**, **Solo Off**. Flip **Source** between **Per channel** and **Mono sum**                    | Per channel keeps the sides apart; Mono sum puts both sides in each double. Note which you prefer | [ can't hear any difference ] |
| 2.8 | **Layers Off**, then **On**                                                                              | Off gives the plain loop, no pop                                                                  | [ y ]                         |

### Stage 3: **Audition 3 Doubles Pitchshift.amxd**

| #   | Do                                                                                         | Expect                                                                                                                   | Pass                       |
| --- | ------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------ | -------------------------- |
| 3.1 | Load it and play anything                                                                  | Within a second, **Doubles PS** on the left changes from "not measured yet" to a number of samples and ms. Write it down | [ 4096 smp 92.9 ms ]       |
| 3.2 | **Sine** with the tuner chain, **Solo Doubles**, **Cents 12**, Utility on Left, then Right | About **+12** cents, then about **-12**, both steady                                                                     | [ y ]                      |
| 3.3 | Step **Quality** through basic, good, better, best                                         | The readout updates each time. Write down each number                                                                    | [ readout isn't updating ] |
| 3.4 | **Loop**, **Solo Off**, Quality **good**                                                   | Doubles in time with the dry loop: no flam                                                                               | [ I don't understand]      |
| 3.5 | Put stage 2's device on a copy of the track and A/B with this one                          | Note which you prefer, and whether you can tell them apart under the dry loop                                            | [ no sure ]                |

### Stage 4: **Audition 4 Doubles Drift.amxd**

| #   | Do                                                                                                                                             | Expect                                                                   | Pass         |
| --- | ---------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------ | ------------ |
| 4.1 | **Sine** with the tuner chain, **Solo Doubles**, **Engine Delay line**, **Cents 12**, **Drift On**, **Depth 3**, **Rate 0.2**, Utility on Left | The reading wanders slowly around +12, by up to about 3 cents either way | [ n ]        |
| 4.2 | Utility on Right                                                                                                                               | Wanders around -12, not in step with the left                            | [ n ]        |
| 4.3 | **Engine Pitchshift**                                                                                                                          | Same, but in small steps (it updates every 20 ms)                        | [ y ]        |
| 4.4 | **Drift Off**                                                                                                                                  | Steady again on both engines                                             | [ just ps ]  |
| 4.5 | **Loop**, **Solo Off**. Try Drift with Depth 1.5 to 6 and Rate 0.1 to 0.5 on both engines                                                      | Note whether it helps, and the settings you liked                        | [ not sure ] |

### Stage 5: **Audition 5 Interval Pitchshift.amxd**

| #   | Do                                                                                       | Expect                                                                    | Pass                                    |
| --- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------------- | --------------------------------------- |
| 5.1 | Load it and play anything                                                                | **Interval PS** on the left shows a number. Write it down                 | [ same as 3.1 ]                         |
| 5.2 | **Sine** with the tuner chain, **Solo Interval**, **Octave down**                        | Tuner reads **A3** (220 Hz), close to 0 cents                             | [ -5cts ]                               |
| 5.3 | **Fifth down**                                                                           | Tuner reads **D4** (293.7 Hz), close to 0 cents                           | [ -2cts ]                               |
| 5.4 | **Loop**, **Solo Interval**, **Octave down**. Step **Quality** through basic to best     | The readout updates each time. Write down each number and how each sounds | [ no change like before]                |
| 5.5 | **Solo Off**, **Level -12**. Judge each Quality under the dry loop                       | Note the lowest Quality that sounds fine                                  | [ seem fine but can't hear any change ] |
| 5.6 | Repeat 5.4 and 5.5 with **Fifth down**                                                   | As above                                                                  | [ seem fine but can't hear any change]  |
| 5.7 | **Wide loop**, **Solo Off**. Flip **Stereo** between **Mono centre** and **Per channel** | Note whether you can hear a difference, and which you prefer              | [ wide loop?]                           |

### Stage 6: **Audition 6 Interval Delay Line.amxd**

| #   | Do                                                                                     | Expect                                                              | Pass                                   |
| --- | -------------------------------------------------------------------------------------- | ------------------------------------------------------------------- | -------------------------------------- |
| 6.1 | **Sine** with the tuner chain, **Solo Interval**, **Octave down**, then **Fifth down** | **A3**, then **D4**, as in stage 5. The reading may flicker more    | [ no difference in sound, no pitching] |
| 6.2 | **Loop**, **Solo Interval**. Try **Window** 30, 60 and 120                             | Shorter is tighter but buzzier, longer is smoother but more smeared | [ same ]                               |
| 6.3 | **Solo Off**, **Level -12**, under the dry loop                                        | Note whether the grain is usable, or even wanted                    | [ same ]                               |
| 6.4 | A/B with stage 5 on a copy of the track                                                | Note which you prefer                                               | [ n/A ]                                |

### Stage 7: **Audition 7 Combined.amxd**

| #   | Do                                                                                                                                                                                                                           | Expect                                                                    | Pass                  |
| --- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------- | --------------------- |
| 7.1 | **Loop**. **Solo Doubles**, then **Solo Interval**, then **Solo Off**, then **Layers Off**                                                                                                                                   | Only the doubles, only the interval, everything, then only the plain loop | [ y ]                 |
| 7.2 | **Solo Off**. Set your favourites from stages 2 to 6. Flip **D Engine** and **I Engine** under the full sound                                                                                                                | Note whether your favourites still win                                    | [ hard to say]        |
| 7.3 | Note your buffer size (**Live > Preferences > Audio**). Read Live's CPU meter (top right) with the device off, then on with each **D Engine** and each **I Engine**, and with the interval on Pitchshift at each **Quality** | Write down the readings                                                   | [ 512 samples, 6-7% ] |
| 7.4 | Optional: duplicate the track (with the device) until the CPU meter reaches about 50%, with your favourite settings                                                                                                          | Write down how many copies                                                | [ 20 tracks and 15%]  |
| 7.5 | Set up your favourite combination                                                                                                                                                                                            | Write down every setting on the panel                                     | [ still unsure ]      |
