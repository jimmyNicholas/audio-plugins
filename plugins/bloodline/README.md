# Bloodline (JUCE spike)

Spike for [#41](https://github.com/jimmyNicholas/audio-plugins/issues/41), part of [Map: Bloodline Breaks](https://github.com/jimmyNicholas/audio-plugins/issues/29). One instrument plugin with one shared drum voice. Kick, Snare and Hats are zones within it. You breed three rows of drums by choosing between pads, all from an MPD226. It is throwaway: it answers whether the breeding loop works as one JUCE instrument, and what the real plugin should change.

What it follows:
- **Voice and zones:** [#43](https://github.com/jimmyNicholas/audio-plugins/issues/43). Eight settings (Pitch, Sweep, Decay, Body / Noise, Noise tone, Metal, Click, Drive). With a zone on, 80% of fresh draws come from it, and mutations outside it are pulled halfway back. The favourite is never pulled.
- **Pads and breeding:** [#37](https://github.com/jimmyNicholas/audio-plugins/issues/37). Notes 36 to 51, three rows of A, B, C and Choose, bottom pads play each row's favourite, and the spread rules.
- **UI:** [#38](https://github.com/jimmyNicholas/audio-plugins/issues/38), Layout C. A bare grid with row tabs, and one inspector with the zone switch, spread and locks.

```
 row 1 (48-51)   [A] [B] [C] [choose]     Hats by default
 row 2 (44-47)   [A] [B] [C] [choose]     Snare by default
 row 3 (40-43)   [A] [B] [C] [choose]     Kick by default
 bottom (36-39)  [row 3] [row 2] [row 1] [empty]
```

## Install on the Mac

1. Open the [Bloodline workflow runs](https://github.com/jimmyNicholas/audio-plugins/actions/workflows/bloodline.yml), pick the latest green run on `claude/plugin-album-collection-5nvyi5`, and download **Bloodline-mac** (you need to be logged in to GitHub).
2. Unzip it (it is a zip inside a zip; unzip both), then in Terminal: `sh install-mac.sh` from that folder. It copies the VST3 and AU into `~/Library/Audio/Plug-Ins` and clears the quarantine flag, because the plugin is not signed.
3. In Live: Preferences, Plug-Ins, make sure "Use VST3 Plug-In System Folders" and "Use Audio Units" are on, then Rescan.
4. Load **one** of the two formats on a MIDI track (Plug-Ins, Jimmy Nicholas, Bloodline). Don't load the VST3 and the AU in the same set.
5. Set the MPD226 to Bank A, so its pads send notes 36 to 51.

Or build it yourself on the Mac (Xcode 15.2, or 14.3.1 on macOS before 13.5, and CMake 3.22 or later):

```
cmake -S plugins/bloodline -B build -G Xcode -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --config Release --target Bloodline_VST3 Bloodline_AU
```

## How to play it

- **Hear:** hit A, B or C in a row. It plays that candidate and underlines it as last pressed.
- **Choose:** hit the row's Choose pad. It commits the last pressed candidate, which becomes the favourite (outlined, starred) and stays on its pad. The other two pads get new sounds bred from it. Choose lights up brighter once there is something to commit, and does nothing otherwise.
- **Spread:** choosing the favourite again narrows the spread. Choosing a new candidate twice in a row widens it. The inspector shows the meter, the value, and a dot for "one new choice so far".
- **Bottom pads:** 36, 37 and 38 play rows 3, 2 and 1's favourite, so a clip on 36 to 38 plays the kit bred so far.
- **Inspector:** click a row's tab (left of the grid). Zone switch: click Kick, Snare or Hats to centre that row there, and click it again (or None) to breed anywhere. Each setting shows the favourite's value, the spread around it as shading, and the zone as a faint line under it. **Lock** keeps that setting at the favourite's value in new candidates.
- **Velocity** sets the level, with a little more click and drive when played harder.
- **Saved with the set:** all three rows (candidates, favourite, spread, zone, locks). A new instance starts from a new random seed.

Notes from a clip count as presses too, so a clip that hits Choose pads will breed.

## Checklist

Play it over a boom bap loop (around 85 to 95 BPM). Post the answers on [#41](https://github.com/jimmyNicholas/audio-plugins/issues/41), with what should change before the plugin set.

**Install and basics**
1. Did it install and load in Live 11 without trouble? VST3, AU, or both? Anything odd with Gatekeeper or the rescan?
2. Do the MPD226's Bank A pads line up with the grid on screen (36 bottom left)?
3. Any clicks, dropouts, CPU spikes or stuck sounds?

**The sound**
4. Starting fresh, how many of the three Kick candidates are usable kicks? Snares? Hats?
5. Best and worst sound you got from each row. What made the worst one bad?
6. Does the voice reach a real boom bap kick (round, short, thumpy), snare (cracky, a bit of body), and hats (closed and open)? What is missing for each?
7. Are Metal and Drive doing anything useful? Is Click too much or too little?
8. Levels: are the three rows roughly even, or does one need turning up or down?

**Breeding**
9. How many choices did it take to get from fresh to a drum you'd keep, per row?
10. The zones and the 80% draw: do Kick, Snare and Hats feel like the right places to start? Too narrow, too wide, wrong somewhere (name the setting)?
11. The halfway pull: try breeding a sound with a zone on, then switch the zone. Does the pull feel like a nudge or a wall? Should it be stronger or weaker?
12. With None on, does breeding anywhere find interesting sounds between drums, or mostly noise?
13. Spread: does narrowing on the favourite again and widening after two new choices feel right? Did you want Wider or Closer back?
14. Did you use locks? Which settings?

**Playing**
15. Did breeding and playing the bottom pads in one sitting work, or did you want to stop the loop to breed?
16. Did a clip ever breed by accident (hitting A, B, C or Choose)?
17. Velocity: does harder playing feel right?

**The UI**
18. Could you tell at a glance which pad is the favourite, which was last pressed, and when Choose will do something?
19. Was the inspector enough, or did you need things on the grid itself?

**For the plugin set**
20. One thing to keep exactly, one thing to cut, one thing to add.

## Building and testing here

The core (`core/`) is plain C++17 with no JUCE: the settings and zones, a breeding `Row`, the shared `Voice`, and the `Instrument` that maps notes to them. The JUCE layer (`plugin/`) only forwards notes, renders, saves and loads, and draws.

```
cmake -S plugins/bloodline -B build-core -G Ninja -DBLOODLINE_PLUGIN=OFF    # core, tests, render; no JUCE
cmake --build build-core && ctest --test-dir build-core --output-on-failure
build-core/bloodline_render <out dir> [seed]                               # zones.wav, candidates.wav, beat.wav
cmake -S plugins/bloodline -B build -G Ninja -DCMAKE_BUILD_TYPE=Release    # everything, fetches JUCE 9.0.5
```

Tests sit at three seams:
- `tests/row_tests.cpp`: breeding, zones, the pull, spread, locks, restore.
- `tests/voice_tests.cpp`: the sound, by numbers.
- `tests/instrument_tests.cpp`: notes in, audio and state out.

On Linux the plugin builds as VST3 and Standalone, and CI runs pluginval at strictness 10 on the VST3. The [workflow](../../.github/workflows/bloodline.yml) also builds the universal Mac VST3 and AU and runs `auval`.

Known shortcuts, fine for a spike:
- The breeding rules here differ from `shared/breeder/engine.js` (no Back, Keep or shuffle, and a spread that widens), so this is a new engine, not a port, and there are no shared test vectors.
- The seed is scattered before Park-Miller sees it, because small seeds give near-zero first draws. The Max engine has the same weakness.
- Metal's squares are naive, so they alias. The audio thread and the editor share one lock.
