# 10: Panel shows the current mode's settings

**Goal:** a tidy panel now that there are six settings for eight modes.

## The problem
After tickets 06 to 09 the panel has Strength, Semitones, Nudge, Length, Range and Ratios, but each mode uses at most one of them:

| Mode | Setting |
| --- | --- |
| Swap | none |
| Gravity, Repel | Strength |
| Move | Semitones |
| Echo | Nudge |
| Rotate | Length |
| Wander | Range |
| Harmony | Ratios |

## Tasks
- Show only the current mode's setting, in one place next to Mode. The others are hidden, not removed, so they still save with the set and can be automated.
  - Hiding uses the patch's scripting names, from `tap.js` (`this.patcher.getnamed(...).hidden`), or from plain Max objects if that misbehaves in Live.
  - Tests: which controls show for each mode, against the fake patcher.
- Check that the panel still fits the device height (169 pixels) and that Reset, Output and Drift view keep their places.
- Optional, if the user wants it: drop this ticket and keep every setting showing.

## Prototype
Change Mode: the setting next to it changes. Automate a hidden setting: it still takes effect when its mode comes up.

## Done when
- Tests pass, including the ES5 check.
- Every setting still survives saving and reopening the set.
