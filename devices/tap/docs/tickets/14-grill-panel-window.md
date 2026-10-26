# 14 (grill): Panel window

**Status:** to grill. Depends on ticket 11 (which controls exist) and ticket 15 (the visualiser section).

**Idea (the user, 2026-10-25):** sliders and knobs that map to a better pop-out window. The window is a panel showing the selected mode, how it works and its modifiers, similar to the web app. The last section is the visualiser (ticket 15).

## Where things stand

- **Device strip:** `live.*` controls drawn by Live in presentation mode, 169 pixels tall. These are the Live parameters: they save with the set and can be automated or mapped to an LFO.
- **Pop-out window:** the Drift view button opens `p Drift` through `pcontrol`. Inside is one `jsui` (`drift-view.js`) that draws everything with `mgraphics` and has its own Keys / Picture switch.
- **Web app:** a mode dropdown, a short description of each mode, the selected mode's modifiers and a Reset button (`src/_components/optionsPanel/Modes.tsx`, descriptions in `src/_lib/_data/modes/`).
- There are two ways to put controls in the window:
  - **A. Real `live.*` controls inside the window's subpatcher.** They are still the device's Live parameters, so saving, automation and mapping all work. Live draws them, so they look like Live.
  - **B. Controls drawn in the `jsui`.** These give full control over the look, but they are not Live parameters. Each one would have to send its value to a hidden `live.*` control, and redraw when that control changes (for example from automation). That is more code, and it is easy to get out of step.

## Decided with the user (2026-10-26, round 1)

- **Every control stays on the strip:** Mode, Mod, Effect Range, Chance, Lowest, Highest, Output, Reset and the window button. The device plays fully without the window.
- **Window controls are real `live.*` controls** (A). Where a control is in both places, the window's copy follows the strip's.
- **How it works:** fixed text per mode, written fresh for the three modes, changing with Mode.
- **Mod is labelled for the selected mode** under its knob (for example Swap "Keys passed round: 3", Gravity "Pull: 3% of the gap", Move "+2.00 semitones").
- **Debug section** behind a Debug toggle in the window, collapsed by default. It holds ticket 11's Gravity tuning and ticket 15's size and header settings.
- **Look:** Live's default for the controls; the visualiser keeps its own colours.
- **Open automatically:** the answer was "yes" to a recommendation of "no". To confirm in round 2.
- **A mock-up first,** to agree the layout before Max work.

## Decided with the user (2026-10-26, round 2)

Against the mock-up at https://claude.ai/artifact/EopL6dHcj3qczfJ9DH9Azs (window, window with Debug open, device strip).

- **Opening:** answered "yes" to the recommendation that it stays a button, read as: it does not open on load.
- **The window repeats every strip control,** each window copy following its strip copy.
- **Layout as in the mock-up:** Mode, How it works, Modifiers and the visualiser side by side, about 920 by 320. Debug opens below, to about 430.
- **How it works:** the mock-up's drafts, for now.
- **The strip button** is called **Open**.
- **Strip layout as in the mock-up:** 360 wide, Mode slider and the Mod, Effect Range and Chance dials at the left; Lowest, Highest and Output at the right; Reset and Open below.
- **One build ticket** for the window, its sections and Debug, with an empty visualiser slot that ticket 15 fills.

## Questions to grill

1. **What stays on the device strip?** All controls (the window repeats them), a few (Mode, the main knob, Reset, the window button), or only the button? *Recommendation:* Mode, Reset and the window button, plus each mode's main knob. The user can play without opening the window.
2. **Controls in the window: A or B?** *Recommendation:* A, real `live.*` controls. If the same parameter must appear both on the strip and in the window, one copy follows the other through `live.remote~` or a hidden link. Say which controls need to be in both places.
3. **"How it works":** fixed text per mode (as in the web app), a small diagram, or an animation of the last release (for example, an arrow from last to current)? *Recommendation:* fixed text first, written fresh for the reworked core three. An animation of the last release could be a later ticket.
4. **Layout:** sections side by side (Mode, How it works, Modifiers, Visualiser), or stacked? What window size? *Recommendation:* side by side in one row, about 900 by 260, with the visualiser square at the right. Ask for a quick sketch.
5. **Only the selected mode's modifiers, or all three modes' at once?** *Recommendation:* only the selected mode's, as in the web app. In Max that means showing and hiding controls in presentation mode when Mode changes.
6. **Keys and Picture views:** settled in ticket 15: the square replaces them. The window needs a slot for it (128 to 512 square, 256 by default), and the debug section needs ticket 15's View size and Header summary settings beside ticket 11's Gravity tuning.
7. **Look:** Live's default look, or a custom colour scheme? *Recommendation:* Live's default for the controls, and the visualiser keeps its own colours.
8. **Open the window automatically when the device loads?** *Recommendation:* no. It stays a button.
9. **Is a first mock-up wanted before Max work?** A static picture (or an HTML sketch) of the window is quick to change. *Recommendation:* yes, one mock-up to agree the layout.

## Done when grilled

- Answers recorded here, with an agreed layout sketch.
- Rewritten as one or more build tickets (the window and its sections, then moving controls off the strip).
