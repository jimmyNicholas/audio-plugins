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
