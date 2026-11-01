# Slush Pitch Layer Audition: how to test it

A throwaway audio effect that runs every candidate way of building the Slush Pitch Layer side by side, with switches to flip between them on the same audio. Your notes settle [How is Slush Pitch Layer built?](https://github.com/jimmyNicholas/audio-plugins/issues/10). Takes about 45 minutes. Post the results on [#14](https://github.com/jimmyNicholas/audio-plugins/issues/14).

It is never released. The build spec is [SPEC.md](./SPEC.md).

## 1. Get the files

1. Switch to the branch `claude/slush-wave-plugin-wayfinding-kyev7g` and pull.
2. Optional: in a terminal, from the repo root, run `npm test`. The last lines should show `# fail 0`.

## 2. Add the folder to Live (once)

1. Open Live 11 and a new empty set.
2. In the browser on the left, under **Places**, click **Add Folder…**.
3. Choose the `devices/slush-wave/prototype` folder inside your `audio-plugins` checkout. It now appears under Places.

## 3. Load the device

1. Create an audio track and put a loop on it (see Setup below).
2. Under **Places > prototype > pitch-layer-audition**, drag **Slush Pitch Layer Audition.amxd** onto the track. Make it the first device on the track.
3. The device shows three groups: **LAYERS** (Layers, Solo and the readouts), **DOUBLES** and **INTERVAL**.
4. Play the loop. You should hear the loop with the doubles and the octave down mixed in.
5. After a second or so, the readouts on the left change from "not measured yet" to a number of samples and ms.

**If no sound comes out or the device shows an error:**

1. Click the **Edit** button in the device's title bar (the small icon at top right). The Max editor opens.
2. In the Max editor, open **Window > Max Console** and look for red lines. See "Troubleshooting" at the end for the ones to expect, then copy every red line into your reply.
3. If Live refuses to load the device at all, copy the message into your reply.

## How the device works

- The dry signal always passes at unity, delayed by a fixed 8192 samples (the device's Defined Latency, so Live compensates for it). Every layer is lined up to the same delay.
- Each layer has two versions (engines). Only the selected one runs; the other is switched off, so Live's CPU meter shows one version at a time.
- When you switch an engine, the old one fades out at once and the new one fades in about a quarter of a second later. That gap is on purpose: it lets old audio left in the new version's buffers play out silently. Judge the sound a moment after a switch, not during it.
- A **Level** dial at its minimum (-70 dB) counts as off: that layer's versions are switched off entirely.
- **Layers Off** always gives the plain, delayed input, whatever Solo says. **Solo Doubles** or **Solo Interval** (with Layers On) mutes the dry signal and the other layer.
- Each `pitchshift~` measures its latency on load, whenever **Quality** changes, and whenever a version is switched on.
- **Trim DL** and **Trim PS** adjust only that engine's level, so you can match the two engines' loudness before comparing them.

## Setup

1. An audio track with a slowed loop: drop in a loop, set its **Warp** mode to **Re-Pitch**, and set the set's tempo to about 0.8 times the loop's own tempo (for a 100 BPM loop, 80 BPM). The loop then plays slower and lower, like a turntable running slow.
2. Put the device first on the track.
3. Optional, for check 1: duplicate the track (**Cmd+D**) and delete the device from the copy, so you have the same loop with nothing on it.
4. Later, for a second pass: a vocal or a single instrument instead of the full loop.

## 4. The checks

Number your answers to match.

### Check 1: alignment (Q8)

1. Set **Layers** to **Layers Off**.
2. Play both tracks (the one with the device and the duplicate without). Toggle the device on and off with the round button at the left of its title bar.
3. **Pass:** the two tracks stay in time, with no flam or echo, with the device on or off.
4. Set **Layers On**, and turn both **Level** dials up to 0 dB.
5. Listen for comb filtering (a hollow, phasey tone) or flamming between the input and the layers.
6. Report: clean / flams / combs. Then put the Levels back (Doubles -9, Interval -12).

### Check 2: doubles engine (Q1)

1. Set **Solo** to **Doubles**, **Cents** to **12**, and **Quality** (in the INTERVAL group) to **good**.
2. Flip **Engine** between **Delay line** and **Pitchshift** a few times. If one is louder, adjust **Trim DL** or **Trim PS** until they match.
3. Repeat at **Cents 5** and **Cents 25**.
4. Set **Solo** to **Off** and flip the engine again under the full sound.
5. Report: which engine you prefer, and whether you can tell them apart in the full sound.

### Check 3: doubles source (Q5)

1. **Solo Off.**
2. Flip **Source** between **Per channel** and **Mono sum**.
3. Report: which is wider or nicer, and anything odd (for example a hole in the middle, or one side louder).

### Check 4: drift (Q7)

1. **Solo Doubles.** Set **Drift** to **Drift On**, **Depth 3**, **Rate 0.2**.
2. Listen for slow wander in the doubles. Then try other settings, for example Depth 6 and Rate 0.5, or Depth 1.5 and Rate 0.1.
3. Try it with both engines. The delay line wanders smoothly; pitchshift steps in small jumps about every 20 ms, so a little roughness there is expected.
4. Report: Off or On, and the settings you liked.

### Check 5: interval engine and quality (Q2)

1. **Solo Interval**, **Interval Octave down**, **Engine Pitchshift**.
2. Step **Quality** through **basic**, **good**, **better** and **best**. After each change, wait a second and write down the **Interval PS latency** readout on the left.
3. Set **Engine** to **Delay line** and try **Window** at **30**, **60** and **120** ms.
4. Set **Solo Off**, **Level -12**, and judge both engines under the full sound.
5. Repeat steps 1 to 4 with **Interval Fifth down**.
6. Report: the lowest Quality that sounds fine, whether the delay line's grain is usable or even wanted, and the latency readout for each Quality.

### Check 6: interval stereo (Q6)

1. **Solo Off**, under the full sound.
2. Flip **Stereo** between **Mono centre** and **Per channel**.
3. Report: can you hear a difference, and which you prefer.

### Check 7: CPU (Q8)

1. Note your buffer size: **Live > Preferences > Audio**, **Buffer Size**.
2. Read Live's CPU meter (top right) with the device off (round button in its title bar).
3. Turn it on, and read the meter with the interval **Engine** on **Delay line**, then on **Pitchshift** at each **Quality**. Do the same for the doubles **Engine**.
4. Optional: duplicate the track (with the device) until the meter reaches about 50%, and count the copies, for the setting you like best.
5. Report: the readings and the buffer size.

### Check 8: overall

1. Set up your favourite combination.
2. Report every setting on the panel.

## 5. What to send back

- Answers to checks 1 to 8.
- The latency readouts (Doubles PS, and Interval PS at each Quality).
- Your Live version (**Live > About Live**) and Max version (top of the Max Console, or **Max > About Max** in the Max editor).
- Any red lines from the Max Console.

## Troubleshooting

The builder could not run Max, and the `pitchshift~` reference page was not reachable, so a few names were taken from search extracts. These are the first places to look.

- **`pitchshift~` attribute or message names.** The patch uses the `pitchshift` attribute (a pitch factor), `quality` (`basic`, `good`, `better`, `best`), `reportlatency 1`, and the `getlatency` message. It expects the latency as `latency <samples>` from the right-hand outlet. A red line like `pitchshift~: doesn't understand ...` names the one that is wrong. Copy it back.
- **Latency readouts stay at "not measured yet".** `pitchshift~` may only report its latency while audio is running. Make sure audio is on and the loop is playing, then change **Quality** and back, or switch that layer's **Engine** away and back; either measures again. If it still says "not measured yet", the latency message is not reaching the patch: open `p doubles-pitchshift` in the Max editor and check what comes out of the right outlet of `pitchshift~`.
- **Quality affects both pitchshift versions.** The menu sits in the INTERVAL group, but the doubles' Pitchshift engine follows it too. Leave it at **good** for checks 2 to 4.
- **Readouts show odd text.** They are built with `sprintf`. If the text looks wrong, copy what the panel shows.
- **`mute~`.** Each version sits behind a `mute~`. If a version keeps using CPU while another is selected, say so.
- **A warning line appears.** It means a measured `pitchshift~` latency is over 8192 samples, so that version can't be lined up with the dry signal. The Max Console has a matching `audition: warning ...` line. Report the number.
- **CPU and Stereo.** Both interval versions always run two shifters, even in Mono centre (the right one is just not heard), so Stereo does not change CPU.
- **Clicks on switching.** Switching an engine has a short gap (see "How the device works"). Switching Source, Stereo or Interval can click; that's expected.

The Max Console lines from the device start with `audition:`, for example `audition: pitchshift interval good latency <n> samples`, with the number in place of `<n>`.

## Notes

- Don't save changes to the device from the Max editor. The repo copy is generated by `build-patch.mjs` and packed with `npm run build` in this folder.
