# Research: building, testing and playing a JUCE instrument from this repo

Researched 2026-11-12 for issue #33 ("How do we build, test and play a JUCE instrument from this repo?"), part of map issue #29 (Bloodline Breaks). Purpose: settle how the JUCE spike (one instrument plugin holding Kick, Snare and Hats, bred by choosing between candidates) gets built, tested in a cloud container, and played in Ableton Live 11 on an Intel Mac running macOS 13. Context: [`business_plan/current-plan.md`](../../business_plan/current-plan.md) (JUCE is Leaning, Windows first, framework path A or B Open) and [`shared/breeder/`](../../shared/breeder/).

## How to read this

- **Tags.**
  - **[verified]**: done in this session's Linux container (Ubuntu 24.04, 4 cores, 15 GB RAM) and seen to work or fail. Scratch work lived in `/tmp/claude-0/juce-spike/`, not in the repo.
  - **[source]**: read directly in a primary source file: the JUCE repository at tag 9.0.5 (cloned in the container), this repo's code, or the GitHub API for this repo.
  - **[extract]**: from search-engine extracts of the cited page, not a full read. The proxy refused `juce.com` and `docs.github.com` (HTTP 403 on CONNECT), so JUCE licence and GitHub billing facts are extracts.
  - **[inference]**: the researcher's reasoning or general knowledge, not checked against a source in this session.
- **Confidence:** High = verified or read in a primary source. Medium = an extract of a maker page, or several secondary sources agreeing. Low = a single forum post, or inference.

## TL;DR

| Question | Answer | Confidence |
|---|---|---|
| Current JUCE | 9.0.5, tagged 2026-11-02, needs CMake 3.22 and C++17 | High [source] |
| Licence | JUCE 9 keeps the JUCE 8 EULA and tiers. Starter is free up to US$20,000 a year revenue or funding. The splash screen code is gone from JUCE 9 | Cap: Medium [extract]. Splash: High [source] |
| Layout | One CMake project: a plain C++17 core library (engine and voices, no JUCE types), a thin JUCE plugin target, and plain test and render executables | High [verified] |
| Formats | VST3 and Standalone build on every platform; AU only on macOS (silently dropped on Linux); LV2 on Linux | High [verified] |
| Getting it onto the Mac | GitHub Actions `macos-15` builds a universal (arm64 + x86_64) VST3 and AU zip; download, copy into `~/Library/Audio/Plug-Ins`, remove the quarantine flag, rescan in Live. Or build locally with Xcode 15.2 (needs macOS 13.5+) or 14.3.1 | Medium |
| CI cost | This repo is **public**, so standard GitHub-hosted runners (Linux, Windows, macOS) are free. If made private: macOS US$0.062 a minute, counted 10x against included minutes | Public: High [source] + Medium [extract]. Prices: Medium [extract] |
| Headless here | Everything except a DAW: JUCE fetched, Linux deps installed, VST3 + LV2 + Standalone built (2 min 25 s), plain C++ unit tests, an offline render to WAV, and pluginval 1.0.4 at strictness 10 including editor tests under Xvfb, all passed | High [verified] |
| Engine port drift | Seeded Park-Miller RNG ports bit-exact. The gaussian (log, cos) differs from V8 by up to 8.9e-16 on about 6% of draws, so shared test vectors need a tolerance on values and exact matching on everything else | High [verified] |
| Keyboard in Live 11 | Make every breeding action a MIDI note. MIDI from a keyboard, clips or Live's Computer MIDI Keyboard always reaches an instrument; computer key presses inside a plugin window are a known mess in Live and depend on focus, format and version | High for MIDI [source]; Medium for the key-press mess [extract] |
| Framework path A or B | The spike can stay neutral if the core is plain C++ with no JUCE types behind a small voice interface. It tilts slightly towards A, because the voices get written in C++ first | Medium [inference] |

## 1. Build

### 1.1 JUCE version and requirements

- The JUCE default branch (`master`) and the newest tag are both **9.0.5**, committed 2026-11-02. Release notes: fixes for native child windows, malformed file protection, Xcode 27 compatibility, Direct2D and VST3 hosting scaling. **[source]** `CHANGE_LIST.md`, `git ls-remote --tags` of https://github.com/juce-framework/JUCE
- Root `CMakeLists.txt` declares `cmake_minimum_required(VERSION 3.22)` and `project(JUCE VERSION 9.0.5 ...)`. **[source]**
- Minimum build requirements from the JUCE README: C++17; **Xcode 12.4** on macOS (Intel macOS 10.15.4); **Visual Studio 2019** on Windows; g++ 7 or Clang 6 on Linux. Deployment targets: macOS 10.11 (x86_64 and arm64), Windows 10 1607. **[source]** JUCE `README.md`
- JUCE can be brought in with `add_subdirectory(JUCE)` (a git submodule or copy) or `find_package(JUCE CONFIG REQUIRED)` after installing it. **[source]** `docs/CMake API.md`. CMake's own `FetchContent` with a pinned tag (`GIT_TAG 9.0.5`, `GIT_SHALLOW TRUE`) does the same as a submodule without the submodule step. **[inference]** The shallow clone is 129 MB. **[verified]**

### 1.2 Licence

- JUCE modules are dual licensed: AGPLv3, or the commercial "JUCE 9 End User Licence Agreement" at `juce.com/legal/juce-9-licence/`. **[source]** JUCE `LICENSE.md`
- The JUCE team's forum announcement "JUCE 9: No pricing or EULA changes" says JUCE 9 keeps the same tier divisions and prices, and the EULA is unchanged from the last JUCE 8 version. **[extract]** https://forum.juce.com/t/juce-9-no-pricing-or-eula-changes/69000
- Starter tier: annual revenue **or funding** up to **US$20,000**. For individuals this counts revenue from all sources, including donations, sponsorship and advertising. **[extract]** https://forum.juce.com/t/revenue-limits-for-juce-tiers/61058, https://forum.juce.com/t/amendments-to-the-juce-end-user-licence-agreement-for-juce-8/61265. So the plan's "US$20k a year" holds, but note the cap is on all revenue and funding, not only plugin sales. **[inference]**
- **No splash screen:** in JUCE 9.0.5, `juce_gui_basics.cpp` only contains a compiler warning that "This version of JUCE does not use the splash screen, the flag JUCE_DISPLAY_SPLASH_SCREEN is ignored". **[source]** The splash requirement was dropped in JUCE 8. **[extract]** forum threads above.
- I could not read the EULA itself (juce.com blocked). Read it once before release, as the README asks. Confidence on the cap: Medium.

### 1.3 Formats

- `FORMATS` accepts `Standalone Unity VST3 AU AUv3 AAX VST LV2`. "`AU` and `AUv3` plugins will only be enabled when building on macOS". **[source]** `docs/CMake API.md`
- Verified: with `FORMATS VST3 AU Standalone LV2` on Linux, the build produced `VST3/`, `LV2/` and `Standalone/` and silently skipped AU, with no configure warning. So one `CMakeLists.txt` serves all three platforms. **[verified]**
- Live 11 on macOS loads both VST3 and AU **[inference]**. Ship both to the Mac, but do not load both versions of the same plugin in one Live Set: Spectrasonics reports mixing them on a Mac in Live can stop plugin windows appearing. **[extract]** https://www.spectrasonics.net/support/knowledgebase/article/view/71/20
- `COPY_PLUGIN_AFTER_BUILD TRUE` installs into the system plugin folders after each local build. **[source]** `docs/CMake API.md`. Handy on the Mac, off in CI.

### 1.4 Recommended layout

Built and run in the container as a scratch spike (`/tmp/claude-0/juce-spike/proj/`) with one toy kick voice and a port of the engine's RNG. **[verified]**

```
plugins/bloodline/              (name to taste; devices/ stays for Max for Live)
  CMakeLists.txt                JUCE via FetchContent pinned to 9.0.5
  core/                         plain C++17, no JUCE headers, no JUCE types
    engine.{h,cpp}              port of shared/breeder/engine.js
    dsp.{h,cpp}                 port of shared/breeder/dsp.js
    voices/kick, snare, hats    each: prepare(sr), setValues(...), trigger(vel), render(float*, n)
  plugin/                       thin JUCE layer: AudioProcessor, editor, state save/load
  tests/
    core_tests.cpp              unit tests of engine and DSP (plain executable, ctest)
    vectors/*.json              produced by the JS engine (see section 4)
    render.cpp                  offline render of a pattern to WAV, for listening and golden files
```

In CMake terms: `add_library(bloodline_core STATIC ...)`, `juce_add_plugin(Bloodline IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE FORMATS VST3 AU Standalone ...)` linking `bloodline_core` and `juce::juce_audio_utils`, and `add_executable` for tests and render, linked only to the core. Set `JUCE_WEB_BROWSER=0` and `JUCE_USE_CURL=0` so the Linux build does not need WebKit or curl. **[verified]**

Why this shape: the core compiles and tests in seconds with no JUCE, no display and no audio device, and the same core is what path A would hand to Emscripten (section 6). **[inference]**

## 2. Where it runs

### 2.1 Building on the user's Intel Mac (macOS 13)

- **Xcode:** Xcode 15.2 is the newest that installs on macOS Ventura and needs **13.5 or later**; Xcode 15.3 needs Sonoma. **[extract]** Apple's table, https://developer.apple.com/support/xcode/. On an earlier 13.x, Xcode 14.3.1 (Ventura 13.0 or later) is the fallback **[extract]** https://developer.apple.com/forums/thread/764826. Both are well above JUCE's Xcode 12.4 floor. **[source]**
- **CMake:** 3.22 or newer, from Homebrew (`brew install cmake`) or the cmake.org installer. **[inference]**
- Build: `cmake -B build -G Xcode -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0` then `cmake --build build --config Release`, or Ninja. Then `auval -v aumu <code> <manufacturer>` checks the AU. **[inference]**, not run (no Mac here).
- A plugin built on the same Mac carries no quarantine flag, so Gatekeeper does not get involved. **[inference]**

### 2.2 Building in GitHub Actions and downloading the artefacts

- **This repo is public** (`"private": false`). **[source]** GitHub REST API for `jimmyNicholas/audio-plugins`. It already runs `.github/workflows/tests.yml` on `ubuntu-latest`. **[source]**
- "GitHub Actions usage is free for standard GitHub-hosted runners in public repositories". Larger runners are always charged. **[extract]** https://docs.github.com/en/billing/managing-billing-for-github-actions/about-billing-for-github-actions. So Linux, Windows and macOS builds cost nothing while the repo stays public. Confidence Medium (docs page not read directly).
- **If the repo goes private** (the plan rejects open sourcing the plugins): since the 1 January 2026 repricing, standard runners cost about US$0.006 a minute (Linux x64), US$0.010 (Windows), **US$0.062 (macOS)**; included minutes are Linux-equivalent, with Windows counted 2x and macOS 10x, so the Free plan's 2,000 minutes is about 200 macOS minutes. **[extract]** https://resources.github.com/actions/2026-pricing-changes-for-github-actions/, https://dev.to/shubhamoriginx/github-actions-changed-pricing-in-2026-heres-exactly-what-youre-paying-now-51db. A clean JUCE plugin build took 2 min 25 s on 4 cores here **[verified]**; a universal macOS build of VST3 and AU might take 5 to 15 minutes, so roughly US$0.30 to 1.00 a build once over the free minutes **[inference]**. That fits the plan's "Mac builds only on release".
- **Runner choice:** `macos-13` was retired on 2025-12-04; `macos-15-intel` is the last x86_64 image, supported to about August 2027. **[extract]** https://github.com/PRQL/prql/pull/5596, https://discuss.python.org/t/dropping-intel-mac-to-tier-2/102100/1. Better: build a **universal binary** on the arm64 `macos-15` runner with `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0`, which runs on the Intel Mac and survives the Intel runner's retirement. **[inference]**
- **Windows:** `windows-latest` with Visual Studio builds the VST3 for a zip, matching the plan's "Windows first, VST3 in a zip". The plan's self-hosted Windows runner on the user's PC also works. **[inference]**
- The workflow uploads `*_artefacts/Release/VST3` and `AU` with `actions/upload-artifact`; the user downloads the zip from the run page while logged in to GitHub. Use path filters so the existing fast `tests.yml` job is not slowed by C++ builds. **[inference]**

### 2.3 Unsigned plugins on macOS: Gatekeeper and quarantine

- Files downloaded through a browser get the `com.apple.quarantine` extended attribute, and Gatekeeper checks quarantined code. An unsigned plugin that is quarantined may refuse to load, show "developer cannot be verified", and "Open Anyway" does not always work for plugin bundles. **[extract]** https://forum.juce.com/t/mac-open-anyway-not-working-for-unsigned-mac-vst3/64574, https://forum.hise.audio/post/96764
- On one's own machine, clear the flag on the installed bundles only:

  ```
  xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"Bloodline.vst3"
  xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"Bloodline.component"
  ```

  then rescan plugins in Live's Preferences (Plug-Ins tab, with the VST3 system folders option on). **[extract]** https://forum.hise.audio/post/115082 for the command; Live's preferences **[inference]**. Do not turn Gatekeeper off system wide (`spctl --master-disable`), which some forums suggest. **[extract]** https://forums.steinberg.net/t/vst3-cannot-be-opened-because-the-developer-cannot-be-verfied/859781
- The arm64 slice of a universal binary must carry at least an ad hoc signature to run on Apple Silicon; Apple's linker adds one automatically. On the Intel Mac this does not matter. **[inference]** Selling on Mac later needs a Developer ID and notarisation (the A$149 a year in the plan). **[extract]** same HISE thread.

## 3. Testing without a DAW

All of this ran in this container. **[verified]**

| Step | Result |
|---|---|
| Fetch JUCE from GitHub (`git clone --depth 1`) | Worked through the proxy (github.com and raw.githubusercontent.com allowed) |
| Toolchain | Already present: cmake 3.28.3, g++ 13.3, clang, ninja, pkg-config, node 22 |
| JUCE Linux packages via `apt-get` (as root) | Worked: `libasound2-dev libjack-jackd2-dev ladspa-sdk libcurl4-openssl-dev libfreetype-dev libfontconfig1-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libxi-dev libegl-dev libglu1-mesa-dev mesa-common-dev`, plus `xvfb`. Skipped `libwebkit2gtk-4.1-dev` by setting `JUCE_WEB_BROWSER=0` |
| Configure and build (Release, Ninja, `-j4`) | VST3, LV2 and Standalone built in 2 min 25 s from clean; build tree 147 MB |
| Core unit tests (plain executable via ctest) | Passed; no JUCE linked |
| Offline render to WAV | 48 kHz 16-bit mono WAV of two kick hits written and read back with Python's `wave` |
| pluginval 1.0.4 (Linux release zip from GitHub) on the VST3 | Strictness 5 headless (`--skip-gui-tests`): SUCCESS. Strictness 10 under `xvfb-run` including Editor and Editor Automation tests: SUCCESS |
| AU validation | Not possible on Linux; pluginval's auval step is a no-op here. Run `auval` on a Mac or in the macOS CI job |
| Steinberg VST3 validator | Skipped by pluginval ("validator path hasn't been set"); it would need building from the VST3 SDK |

Package list source: **[source]** JUCE `docs/Linux Dependencies.md` (which says it was tested on Ubuntu 16.04 to 20.04; it worked on 24.04 here). pluginval: https://github.com/Tracktion/pluginval, latest stable tag v1.0.4 (v2.0.0 betas exist). **[source]**

Blocked or not tried:
- `juce.com`, `docs.github.com`: proxy refused (403). The GitHub REST API for `Tracktion/pluginval` was refused (repo not attached to the session), but the public release download URL worked.
- Emscripten: not tried.
- Real audio or MIDI devices, and any DAW: not available in a container. Hosting the plugin in a small JUCE host and feeding it MIDI offline is possible but was not built. **[inference]**

What this means for CI: the Linux job can run core tests, the render, the plugin build and pluginval (under Xvfb) on every push; macOS and Windows jobs only need to build, run `auval` (macOS) and upload. **[inference]**

## 4. Porting the breeding engine and keeping it from drifting

### 4.1 What there is to port

- `engine.js` (137 lines) is pure and ES5: `create`, `choose`, `setStep`, `back`, `reset`, `setLock`, `setShuffle`, `breedFrom`, with a Park-Miller RNG, Box-Muller gaussian, `fold` and a shuffle. State is a plain object deep-copied with `JSON.parse(JSON.stringify())`. **[source]** [`shared/breeder/engine.js`](../../shared/breeder/engine.js). In C++ this maps to a value-type `struct State` copied by value and a free-function API. **[inference]**
- `dsp.js` (142 lines) holds parameter scaling, envelope breakpoints for `line~`, envelope compression, and the loudness simulation (noise, SVF, weighting, matching). **[source]** [`shared/breeder/dsp.js`](../../shared/breeder/dsp.js)
- The sound itself is **not** in JavaScript: each device's voice is MSP objects in its patch, driven by `line~` breakpoint lists. So the C++ voices are a rewrite that reads the same breakpoints per sample (`valueAt`, `stepper` already exist in `dsp.js`), not a port. **[source]** [`shared/breeder/README.md`](../../shared/breeder/README.md); **[inference]** on the consequence.

### 4.2 What was verified about drift

- **Seeded RNG ports bit-exact.** The JS engine's `create()` was run for five seeds (1, 7, 12345, 99999, 2147483646) with 8 parameters, writing 17-digit values to a vectors file; a C++ port read it and compared with `!=`: **0 mismatches**. The product `seed * 16807` stays below 2^53, so plain doubles are exact in both, as the engine's comment says. **[verified]**
- **The gaussian does not.** Over 100,000 draws from seed 12345, **5,909 (5.9%)** differed between V8 and glibc, by at most **8.9e-16**, at both `-O0` and `-O2`. The cause is `Math.log` and `Math.cos` (V8's maths library versus glibc's), which IEEE 754 does not require to be correctly rounded. **[verified]**
- Because the RNG state is integer-exact and is never fed back from the gaussian, these differences never compound: the same picks give the same random stream, the same shuffle order and the same candidate ids, and values differ by an ulp or so. **[inference]** from reading the code.

### 4.3 Keeping both in step while both exist

1. **Shared test vectors, produced by the JS engine.** A Node script in `shared/breeder/tests/` runs scripted sessions (create, choose sequences, setStep, back, locks, shuffle on and off, reset, breedFrom) and writes JSON with numbers printed to 17 significant digits. Commit the JSON. The repo's `npm test` regenerates and diffs it (so a JS change without updated vectors fails); the C++ `ctest` reads the same file. **[inference]**, prototype verified above.
2. **Match exactly** on integers and structure (seed, round, ids, favourite flags, slot order, locks, picks) and **within a tolerance** (say 1e-12 absolute) on values and step. **[inference]** based on the measured 8.9e-16.
3. **Same for DSP:** golden envelopes and short renders from `dsp.js` maths compared within tolerance; audio renders compared by RMS and peak, not sample for sample. **[inference]**
4. **Pick a source of truth.** Max 8's `js` object only understands ES5 (the CI enforces it), so the JS cannot be replaced by WebAssembly built from the C++ for the Max for Live devices **[source]** `.github/workflows/tests.yml`; **[inference]** on the consequence. Keep JS as the reference until the Max for Live spikes are retired, then let the C++ be the reference and drop the vectors' JS producer.

## 5. Keyboard control inside Live 11

### 5.1 MIDI notes: reliable

- A JUCE plugin declared `IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE` receives note events in `processBlock`'s `MidiBuffer`, sample-accurate. **[source]** `docs/CMake API.md`; the scratch processor triggers on note-on and passed pluginval's processing tests **[verified]**, though no real MIDI keyboard was involved.
- In Live, an instrument gets notes from clips, from a MIDI keyboard, or from Live's **Computer MIDI Keyboard** (toggle with the M key; off by default when loading a Set) when the track is armed. While it is on, it takes precedence over Live's single-letter shortcuts. **[extract]** https://help.ableton.com/hc/en-us/articles/360000040744-Computer-MIDI-Keyboard-off-by-default, https://www.ableton.com/en/live-manual/11/midi-and-key-remote-control. The middle letter row plays the white keys of one octave, so in Live's default octave A S D F G H J are C3 to B3. **[inference]**, layout not re-read in the manual.
- The Max for Live breeders already work this way: C3, D3, E3 play slots A, B, C, and F3 to B3 are the four save slots. **[source]** [`shared/breeder/glue.js`](../../shared/breeder/glue.js). Those are exactly the white keys of Live's default Computer MIDI Keyboard octave **[inference]**, so the same scheme carries straight into the plugin.

### 5.2 Computer key presses in the plugin window: unreliable

- JUCE's `EDITOR_WANTS_KEYBOARD_FOCUS` defaults to `FALSE` ("defer all keyboard handling to the host"). **[source]** `docs/CMake API.md`. In the AU wrapper on macOS, a key the editor does not use is re-sent to the host view (`[hostView keyDown: currentEvent]`). **[source]** `juce_audio_plugin_client_AU_1.mm`
- Live's own help says some plug-ins block computer keyboard input while their window is open, and to close plug-in windows to get shortcuts back. **[extract]** https://help.ableton.com/hc/en-us/articles/360000093570-Keyboard-shortcuts-not-working
- JUCE forum reports: Live "eats" key events from VST plugins; after clicking into a plugin on macOS it can steal the space bar, tab and musical keys from Live; Tab passes through differently in AU and VST3; in Live 11.3.4 on Windows one key press arrived twice. Returning `false` from `keyPressed` passes the key back, `true` keeps it until the user clicks outside. No official fix. **[extract]** https://forum.juce.com/t/64-bit-vst-eats-keys-in-ableton-live/12801, https://forum.juce.com/t/ableton-live-receives-tab-keypresses-from-au-but-not-vst-vst3/57525, https://forum.juce.com/t/component-keypressed-issue-duplicated-event-in-ableton-event-is-not-consumed-even-with-return-true-in-studio-one/57874, https://forum.juce.com/t/au-plugin-not-passing-keypress-to-live/5244
- Confidence Medium: forum reports across years and formats, none tested here.

### 5.3 What other plugins do, and the recommendation

- Sample players and drum instruments commonly use **keyswitches** (notes outside the playing range that change state) for hands-on control, because MIDI reaches the plugin whatever has focus. **[inference]**
- **Recommendation:** every breeding action (choose A, B or C, Back, save to a slot, maybe Variation up and down) is a MIDI note on a fixed key, shown on the UI, so Live's Computer MIDI Keyboard or any controller drives it with the plugin window open or closed. The mouse UI does the same actions. Optional in-window shortcuts can come later, returning `false` for every key they do not use, and must be tried in Live 11 on the Mac in both VST3 and AU before they are relied on. **[inference]**
- One design question for the spike: a note that chooses also changes state, so choice notes should be ignored during clip playback or on a separate MIDI channel, so a sequenced beat cannot breed by accident. **[inference]**

## 6. Does the spike force framework path A or B?

- Path A: plain C++ for the DSP, compiled with Emscripten for the browser and wrapped in JUCE for the plugin. Path B: Faust or Cmajor exporting to both. **[source]** [`business_plan/current-plan.md`](../../business_plan/current-plan.md)
- The spike can stay **neutral** if the core (section 1.4) has no JUCE headers or types, takes `float*` buffers and plain structs, and each voice sits behind a small interface (`prepare`, `setValues`, `trigger`, `render`). Under path A that core is what Emscripten compiles. Under path B a voice's insides are replaced by Faust-generated C++ (Faust's C++ backend emits a class with a `compute` method) or Cmajor output, behind the same interface. **[inference]**
- The breeding engine is control logic, not signal processing; it stays plain C++ (or JS in the browser) under both paths, so it is not part of the A/B choice. **[inference]**
- It tilts towards A in practice: writing the three voices in C++ first means path B later rewrites them. The plan's "try one tiny effect in both paths" still decides it; the spike only commits to the interface. **[inference]**
- Verified here: the core library built and tested with no JUCE linked. **[verified]** Emscripten and Faust were not tried.

## Sources

Primary, read directly:
- JUCE repository at tag 9.0.5: https://github.com/juce-framework/JUCE (`README.md`, `LICENSE.md`, `CHANGE_LIST.md`, `docs/CMake API.md`, `docs/Linux Dependencies.md`, `modules/juce_gui_basics/juce_gui_basics.cpp`, `modules/juce_audio_plugin_client/juce_audio_plugin_client_AU_1.mm`)
- pluginval: https://github.com/Tracktion/pluginval (release `pluginval_Linux.zip`, v1.0.4)
- This repo: `shared/breeder/*.js`, `shared/breeder/README.md`, `.github/workflows/tests.yml`, `business_plan/current-plan.md`, GitHub REST API repo metadata

Extracts (pages not read in full):
- https://forum.juce.com/t/juce-9-no-pricing-or-eula-changes/69000
- https://forum.juce.com/t/revenue-limits-for-juce-tiers/61058
- https://forum.juce.com/t/amendments-to-the-juce-end-user-licence-agreement-for-juce-8/61265
- https://docs.github.com/en/billing/managing-billing-for-github-actions/about-billing-for-github-actions
- https://resources.github.com/actions/2026-pricing-changes-for-github-actions/
- https://dev.to/shubhamoriginx/github-actions-changed-pricing-in-2026-heres-exactly-what-youre-paying-now-51db
- https://github.com/PRQL/prql/pull/5596
- https://discuss.python.org/t/dropping-intel-mac-to-tier-2/102100/1
- https://developer.apple.com/support/xcode/
- https://developer.apple.com/forums/thread/764826
- https://forum.juce.com/t/mac-open-anyway-not-working-for-unsigned-mac-vst3/64574
- https://forum.hise.audio/post/96764
- https://forum.hise.audio/post/115082
- https://forums.steinberg.net/t/vst3-cannot-be-opened-because-the-developer-cannot-be-verfied/859781
- https://www.spectrasonics.net/support/knowledgebase/article/view/71/20
- https://help.ableton.com/hc/en-us/articles/360000040744-Computer-MIDI-Keyboard-off-by-default
- https://help.ableton.com/hc/en-us/articles/360000093570-Keyboard-shortcuts-not-working
- https://www.ableton.com/en/live-manual/11/midi-and-key-remote-control
- https://forum.juce.com/t/64-bit-vst-eats-keys-in-ableton-live/12801
- https://forum.juce.com/t/ableton-live-receives-tab-keypresses-from-au-but-not-vst-vst3/57525
- https://forum.juce.com/t/component-keypressed-issue-duplicated-event-in-ableton-event-is-not-consumed-even-with-return-true-in-studio-one/57874
- https://forum.juce.com/t/au-plugin-not-passing-keypress-to-live/5244
