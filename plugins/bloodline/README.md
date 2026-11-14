# Bloodline (JUCE spike)

The second spike, for [#47](https://github.com/jimmyNicholas/audio-plugins/issues/47), part of [Map: Bloodline Breaks](https://github.com/jimmyNicholas/audio-plugins/issues/29). One instrument plugin with one shared drum voice, bred by choosing between pads, all from an MPD226. The first spike ([#41](https://github.com/jimmyNicholas/audio-plugins/issues/41)) had three fixed breeding rows; this one has a **workbench** you breed on and a **kit** of three slots you place rows into. It is throwaway: it answers whether the workbench and kit, with a wider voice you can turn by hand, make a better drum instrument than the first spike.

What it follows:
- **Workbench and kit:** [#44](https://github.com/jimmyNicholas/audio-plugins/issues/44#issuecomment-6870956752). Open and placed rows, Place, Random, moving, Reset, Clear, Start again, saving.
- **The voice:** [#45](https://github.com/jimmyNicholas/audio-plugins/issues/45#issuecomment-6871180603). 19 settings in five groups (Body, Noise, Click, Grit, Out), turned by hand on the last played pad, pad Pan and Level with a row offset, and Mono on the row and on each pad.
- **Zones:** [#43](https://github.com/jimmyNicholas/audio-plugins/issues/43), now centring only Body, Noise and Click. Every row starts with None.
- **UI:** [#38](https://github.com/jimmyNicholas/audio-plugins/issues/38), Layout C, grown into the kit.

```
 kit slot 3 (48-51)   [A] [B] [C] [random | choose]     blank: [ ] [ ] [ ] [place]
 kit slot 2 (44-47)   [A] [B] [C] [random | choose]
 kit slot 1 (40-43)   [A] [B] [C] [random | choose]
 workbench  (36-39)   [A] [B] [C] [choose]
```

## Install on the Mac

1. Open the [Bloodline workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline.yml), pick the latest green run on `claude/plugin-album-collection-5nvyi5`, and download **Bloodline-mac** (you need to be logged in to GitHub).
2. Unzip it (it is a zip inside a zip; unzip both), then in Terminal: `sh install-mac.sh` from that folder. It copies the VST3 and AU into `~/Library/Audio/Plug-Ins` and clears the quarantine flag, because the plugin is not signed.
3. In Live: Preferences, Plug-Ins, make sure "Use VST3 Plug-In System Folders" and "Use Audio Units" are on, then Rescan.
4. Load **one** of the two formats on a MIDI track (Plug-Ins, Jimmy Nicholas, Bloodline). Don't load the VST3 and the AU in the same set. It replaces the first spike, which has the same name.
5. Set the MPD226 to Bank A, so its pads send notes 36 to 51.

Or build it yourself on the Mac (Xcode 15.2, or 14.3.1 on macOS before 13.5, and CMake 3.22 or later):

```
cmake -S plugins/bloodline -B build -G Xcode -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --config Release --target Bloodline_VST3 Bloodline_AU
```

## How to play it

- **Breed on the workbench** (36 to 39, the bottom row): hit A, B or C to hear it, then Choose (39) to commit it. The chosen pad becomes the favourite (outlined, starred) and stays; the other two are bred from it. Choosing the favourite again narrows the spread; choosing a new pad twice in a row widens it.
- **Place:** when you like the bench row, hit the fourth pad of a blank slot (43, 47 or 51). The row moves into that slot, placed, and the workbench starts completely fresh (None, starting spread, no locks). You can place before choosing anything.
- **Play a placed row:** A, B and C play its three pads; the fourth pad is **Random**, a different one of the three on every hit. A clip on a slot's notes plays whatever is in that slot.
- **Open a placed row** (Open, beside the slot) to breed it again in place: its fourth pad becomes Choose. Close turns it back into Random. Several rows can be open at once.
- **Move** (up and down arrows) swaps a row with the slot above or below, blank or not. The workbench does not move. **Clear** empties a slot. **Reset** (open rows only) gives three fresh pads and keeps zone and locks. **Start again** (top, click twice) empties the kit and refreshes the workbench.
- **Inspector:** click a line's tab (left). It shows the zone switch, the spread, the row's Pan, Level and Mono, then five groups that open and close (Body is open at first). The settings show the **last played** pad. On an open row you can drag any setting by hand, lock it, or switch the pad's Mono. Turning does not lock a setting and does not change the spread: an unlocked value you turned is bred over in the next round. A placed row shows its settings greyed out; its Pan, Level and Mono still work, like a mixer.
- **Zones:** Kick, Snare or Hats centre Body, Noise and Click; click the zone that is on (or None) to breed anywhere. Choosing Kick switches the row's Mono on; switch it off if you like. Grit, the filters and Pan always breed over their full range.
- **Level** is never bred and starts at 0 dB on every pad, so a quiet pad never sounds like a bad one. Turn it by hand.
- **Velocity** sets the level, with a little more click and drive when played harder.
- **Saved with the set:** the workbench and every slot (pads, favourite, spread, zone, locks, Pan, Level, Mono, open or placed), and which inspector groups are open. Random's last pick is not. A new instance starts from a new random seed. Sets saved with the first spike do not load into this one.

Notes from a clip count as presses too, so a clip that hits a Choose or Place pad will breed or place.

With None and the wide ranges, many fresh pads will be noise, near-silence or something harsh. That is accepted (#45): Reset or a zone is the way out.

## Checklist

Play it over a boom bap loop (around 85 to 95 BPM). Post the answers on [#47](https://github.com/jimmyNicholas/audio-plugins/issues/47), with what should change before the plugin set.

**Install and basics**
1. Did it install and load in Live 11 without trouble? Anything odd with Gatekeeper or the rescan?
2. Do the MPD226's Bank A pads line up with the grid on screen (36 bottom left, the workbench)?
3. Any clicks, dropouts, CPU spikes or stuck sounds? Any pad that goes painfully loud?

**Workbench and kit**
4. Building a kit: breed on the bench, place, breed the next. Does that loop feel better or worse than the first spike's three fixed rows?
5. Random on a placed row: is a different sibling every hit what you want in a beat? Did you want weighting towards the favourite after all?
6. Did you open placed rows to breed them again? Did opening in place work, or did you want to send a row back to the bench?
7. Moving rows and the sound following the slot: useful, or did it break a clip you had?
8. Reset, Clear and Start again: anything missing, or anything you hit by accident?
9. **Zone and locks only while a row is open:** is that right, or should a placed row's zone and locks be editable too?

**The voice**
10. Starting from None, how often is a fresh pad usable? With a zone on?
11. Which of the new settings earn their place (Sweep time, Attack, Noise decay, Click tone, Bits, Rate, High-pass, Low-pass, Resonance, Pan)? Which would you cut?
12. Are the ranges wide enough to be interesting, or too wide to breed in? Name the setting.
13. The drafted zone numbers for the new settings: do Kick, Snare and Hats still start in the right places?
14. Turning by hand: did you use it? Did a turned favourite breed the way you expected?
15. Pan and Mono: does the Kick zone switching Mono on feel right? Did bred pan make the kit too wide?
16. Levels: did Level at 0 dB and the row Level offset do what you needed?

**The UI**
17. Could you tell at a glance which rows are open, placed or blank, and which pad is the favourite and the last played?
18. The collapsible groups: is Body the right one to start open? Is the inspector too long?
19. Anything you wanted on the MPD226 that is only on screen (or the other way round)?

**For the plugin set**
20. One thing to keep exactly, one thing to cut, one thing to add.

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the settings, groups and zones, a breeding `Row`, the shared stereo `Voice`, and the `Instrument` that holds the workbench and kit and maps notes to them. The JUCE layer (`plugin/`) only forwards notes, renders, saves and loads, and draws.

```
cmake -S plugins/bloodline -B build-core -G Ninja -DBLOODLINE_PLUGIN=OFF    # core, tests, render; no JUCE
cmake --build build-core && ctest --test-dir build-core --output-on-failure
build-core/bloodline_render <out dir> [seed]                               # zones.wav, candidates.wav, beat.wav (stereo)
cmake -S plugins/bloodline -B build -G Ninja -DCMAKE_BUILD_TYPE=Release    # everything, fetches JUCE 9.0.5
```

Tests sit at three seams:
- `tests/row_tests.cpp`: breeding, zones, the pull, spread, locks, Random, reset, Level, Mono, turning by hand, restore.
- `tests/voice_tests.cpp`: the sound, by numbers.
- `tests/instrument_tests.cpp`: notes and actions in (place, open, move, reset, clear, Start again), audio and state out.

On Linux the plugin builds as VST3 and Standalone, and CI runs pluginval at strictness 10 on the VST3. The [workflow](../../.github/workflows/bloodline.yml) also builds the universal Mac VST3 and AU and runs `auval`.

Known shortcuts, fine for a spike:
- The breeding rules here differ from `shared/breeder/engine.js` (no Back, Keep or shuffle, and a spread that widens), so this is a new engine, not a port, and there are no shared test vectors.
- The seed is scattered before Park-Miller sees it, because small seeds give near-zero first draws. The Max engine has the same weakness.
- Metal's squares are naive, so they alias, and so does the Rate crusher (on purpose). The audio thread and the editor share one lock.
- A soft knee above -3 dB on each voice keeps Resonance near self-oscillation from clipping hard; several loud pads at once can still reach full scale.
- The inspector's open groups are read by the host's save without a lock.

## Notes from playing the first spike

Posted by the user after playing [#41](https://github.com/jimmyNicholas/audio-plugins/issues/41), kept as written. This spike answers 2 to 8 and 10; 9 is [#46](https://github.com/jimmyNicholas/audio-plugins/issues/46), with a first go at it in [`plugins/fx-breeder/`](../fx-breeder/).

1. It works fine as far as I can tell. I skipped the questions above because I'm driving this.
2. It needs a reset option
3. I liked that I could play all three versions in the drum roll. I ended up using nearly all of them to make a 1 bar loop. The similarity was nice
4. I can see the bottom row being [A][B][C][choose]. This row is the current row being worked. The row above is blank [ ] [ ] [ ] [ bring selected row here]. After the selected row is placed into the row above that row becomes [ A ] [ B ] [ C ] [ random A , B, or C]
5. If the row is opened in edit mode again. the [random] becomes [choose]
6. The settings should be able to be adjusted by the user. We need a pan and high pass / low pass option. Actually, a much bigger range of options. They shouldn't be effects though. That's seperate
7. Each new opening row should start in [none].
8. The rows should be able to move up and down.
9. I would like to see a seperate version that is just for effects. Reverb, delay, compression, saturation, filter, widening, limiting. These are can be added to a drum pad individually or to the whole bus. The chain is also part of the randomisation
10. Things need to stay experimental so ranges should fall outside recommended norms.
