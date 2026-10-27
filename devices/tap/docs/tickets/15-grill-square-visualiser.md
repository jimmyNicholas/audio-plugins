# 15: Square visualiser

**Status:** built 2026-10-27, not yet tested in Live. Grilled 2026-10-26. Ticket 14 made the panel window, its debug section and the slot for this view.

**Goal:** a square pitch map of all 128 keys in the panel window, replacing the Keys and Picture views. The user's idea was a square version of the drift view, 128 by 128, where drift makes a key's line longer.

## The view

- **A pitch map on a true linear scale.** Across is the key, note 0 at the left. Up is the pitch it plays, note 0 at the bottom. At concert pitch every key sits on the diagonal from bottom left to top right.
- **Size:** a square of 128, 256, 384 or 512 pixels, so each note is a whole number of pixels (1 to 4). It opens at 256.
- **Drawing:** a black background. Each key is one note-sized pixel on the diagonal:
  - **White** at concert pitch: drift under 0.1 cent, the smallest step `tap.js` sends.
  - **Full red** when sharp and **full blue** when flat, at any drift. There is no shading by distance.
  - **A stem** in the same colour runs from the diagonal pixel to the exact pitch the key plays. Drift too small for a stem shows as colour only.
- **Header summary line** ("12 keys drifted" or "Every key at concert pitch"): off by default.
- It jumps to the new state on each change. No animation, no hover.
- No bounds, last key or held keys are shown, so `tap.js` does not change.

## Debug settings

From ticket 14's debug section. Neither is a Live parameter or saved with the set; both reset on load, like ticket 11's Gravity tuning.

- **Size:** 128, 256, 384 or 512. Once the user picks one in Live, it becomes fixed and the setting goes.
- **Header summary:** on or off.

## Tasks

- `drift-view.js`, test-first:
  - Delete the Keys and Picture views: the keyboard, bars, landscape, buttons, hover and their constants.
  - `squareLayout(size)`: pixels per note (`size / 128`) and the square's position under the header, when it is shown.
  - `keyRect(note, cents, layout)`: the diagonal pixel for a key, and `stemRect` from it to the pitch played (`note + cents / 100`), or none when shorter than one pixel.
  - `keyColour(cents)`: white under 0.1 cent, else full red or blue.
  - Max handlers `size` (snaps to 128, 256, 384 or 512) and `summary` (0 or 1), each redrawing the view. `list` stays as it is.
- Tests (`tests/drift-view.test.js`):
  - Drop the Keys and Picture tests.
  - Concert pitch draws 128 white pixels on the diagonal and no stems.
  - Move +3 on note 60 at size 256: a red pixel at note 60's diagonal place and a red stem 6 pixels up.
  - A Swap pair: one red stem up, one blue stem down, of equal length.
  - 0.1 cent sharp turns the pixel red with no stem; 0.05 cents stays white.
  - Each size gives 1 to 4 pixels per note, and other sizes snap to the nearest.
  - The header is hidden by default and shows the summary when turned on.
  - The ES5 check still passes.
- Patch: ticket 14 puts this `jsui` in the window and wires the debug section's `size` and `summary` messages to it. Regenerate `TAP.amxd` if the patch changes.

## Prototype

1. `git pull`, then `npm test --prefix m4l`. The last lines show `# fail 0`.
2. Open the panel window. The visualiser shows a white diagonal on black, 256 square, with no header.
3. **Move,** Mod 3: release middle C. Its pixel turns red with a stem 3 notes up.
4. **Move,** Mod 0.01: release a key. Its pixel turns red with no visible stem.
5. **Swap,** Mod +1: release A then B. A red stem up and a blue stem down appear.
6. **Gravity,** Mod 2: keys turn red and blue with short stems.
7. **Reset** and a mode change put the white diagonal back.
8. **Debug:** try each size, and turn the header on and off. Pick a size to keep.
9. **Speed:** fast passages with the window open feel as tight as before.

## Done when

- Tests pass, including the ES5 check.
- The user has played all three modes with the view open in Live and picked a size.

At the end of each ticket, open a pull request into main.
