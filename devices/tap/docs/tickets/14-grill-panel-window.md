# 14: Panel window

**Status:** built 2026-10-27. `npm test` passes. Not yet tested in Live. Ticket 15 then fills the visualiser slot.

**Goal:** a pop-out panel window showing the selected mode, how it works, every control and (from ticket 15) the visualiser, like the web app's options panel. The strip keeps every control, so the device plays fully without the window.

Mock-up: https://claude.ai/artifact/EopL6dHcj3qczfJ9DH9Azs (window, window with Debug open, device strip).

## The strip

- 360 wide, 169 tall, Live's own look.
- At the left: the Mode slider (Swap, Gravity, Move), then the Mod, Effect Range and Chance dials in a row, then Reset and **Open**.
- At the right: Lowest, Highest and Output.
- **Open** replaces Drift view. It is not a Live parameter. The window never opens on its own when the device loads.

## The window

About 920 by 320, four sections side by side:

1. **Mode:** the Mode slider, Reset and Output.
2. **How it works:** the mode's name and a short text that changes with Mode (the mock-up's drafts, for now):
   - **Swap:** "Each release passes pitches round the keys you let go most recently. Mod sets how many: +1 swaps two keys, +2 rotates three, up to thirteen. A negative Mod turns the other way."
   - **Gravity:** "Each release pulls the key let go before it towards the one you just let go, by a share of the gap. Each doubling of Mod triples the pull. A negative Mod pushes them apart."
   - **Move:** "Each release moves the key you let go by Mod semitones. Fractions give microtones, and a negative Mod moves it down."
3. **Modifiers:** the Mod, Effect Range and Chance dials, Lowest and Highest, and a line under Mod saying what it does in the selected mode:
   - **Swap:** "Keys passed round: N" (N from ticket 11's count; "No swap" when it rounds to 0).
   - **Gravity:** "Pull: X% of the gap", or "Push: X% of the gap" for a negative Mod, from ticket 11's share curve.
   - **Move:** "+2.00 semitones each release".
4. **Visualiser:** an empty black 256 square, filled by ticket 15.

Under the sections, a **Debug** toggle, collapsed by default. Open, the window grows to about 430 and shows:

- **Gravity tuning:** pull at Mod 1 (default 1%) and the factor per doubling (default 3), from ticket 11.
- **View size** (128, 256, 384, 512) and **Header summary** (off or on), for ticket 15.
- None of these are Live parameters or saved with the set. They reset when the device loads.

## How the window's controls follow the strip

- The strip's controls stay the device's Live parameters, so saving, automation and mapping are unchanged.
- Each window copy is the same `live.*` object with its parameter turned off. Turning a window copy sets its strip control; a change to a strip control (by hand, automation or an LFO) sets the window copy with `set`, so it does not send the value back.

## Tasks

- `tap.js`, test-first:
  - `modLabel()`: the text under Mod for the current mode and Mod, as above. A third outlet sends it on every change to Mode or Mod, and on `sync`.
  - Gravity tuning: `setGravityAtOne` and `setGravityPerDoubling`, with Max handlers, replacing ticket 11's two constants as defaults. They change no pitch and are not read by `sync`.
- Tests:
  - The label for each mode: Swap counts (including "No swap"), Gravity pull and push at the Mod values in ticket 11's table, Move with signs and two decimals.
  - The label is sent on Mode and Mod changes and on `sync`, not on releases.
  - Gravity tuning changes the share: at pull 2% and factor 2, Mod 4 gives 8%.
  - The ES5 check still passes.
- Patch:
  - Rename `p Drift` to `p Panel` and lay out the window as above, in presentation mode, with its window size set. Keep `pcontrol`, and the `---` sends so two devices each get their own window.
  - The strip as above, with **Open** in place of Drift view.
  - A window copy for each control (Mode, Mod, Effect Range, Chance, Lowest, Highest, Output, Reset), parameter off, linked both ways as above.
  - How it works: a `live.comment` or `comment` whose text changes with Mode.
  - The Mod label from `tap.js`'s third outlet into a comment under Mod.
  - The Debug toggle shows and hides the debug controls and resizes the window. Gravity tuning goes into the `js`; View size and Header summary go to the visualiser slot, ready for ticket 15.
  - Update the patch tests: every strip control is still wired through its `prepend`, every window copy has its parameter off and is linked to its strip control, and Drift view is gone.
  - Regenerate `TAP.amxd`.
- Remove the old Drift view's window. Its `jsui` is rewritten in ticket 15; until then the slot stays empty.

## Prototype

The patch changes, so drag the device in again.

1. `git pull`, then `npm test --prefix m4l`. The last lines show `# fail 0`.
2. **Strip:** matches the mock-up, with **Open** in place of Drift view. Everything plays as before without opening the window.
3. **Open:** the window opens at about 920 by 320, with the four sections. It does not open by itself when the set loads.
4. **Follow:** turn Mod on the strip; the window's Mod follows. Turn it in the window; the strip follows. Do the same for each control.
5. **Automation and LFO:** an LFO on Mod moves both copies, with no jumps or loops. Automation lanes list each control once.
6. **How it works and the label:** change Mode; the text changes. Turn Mod in each mode and read the label: Swap counts keys, Gravity gives a percentage (pull or push), Move gives semitones.
7. **Debug:** click Debug; the window grows and shows the tuning, size and header settings. Set the pull at Mod 1 to 2%; Gravity at Mod 1 pulls twice as fast, and the label reads 2%. Reload the device; Debug is back to its defaults.
8. **Saved with the set:** save, close and reopen. The strip's values come back and the window copies match them when opened.

## Done when

- Tests pass, including the ES5 check and the patch checks.
- The user has opened the window in Live, used every control from both places, and tried Debug.

At the end of each ticket, open a pull request into main.
