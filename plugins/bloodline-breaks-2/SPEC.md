# Bloodline Breaks 2: second pass (spec)

**For:** a fresh session building on the prototype in this folder (commit `1af55fc`). Read [README.md](README.md) first: it describes what exists. This spec lists only what changes. Everything not mentioned here stays as it is.

**Still throwaway.** Same plugin, same folder, same plugin ID (`Brk2`). Bump the saved state to `bloodline-breaks-2 2`; version 1 states are rejected and the plugin starts fresh, which is fine for a prototype.

**The question it answers:** with the user's notes from the first pass fixed, does breeding a kit and loops that play it, in one window, feel like one instrument?

## The user's notes, and what each one becomes

| Note | Change |
| --- | --- |
| Zone and style have no [None] | 1. None options |
| Spread should be adjustable, and reset on [New] | 2. Spread the user can drag |
| The right panel's grid is too rigid; dragging off the grid should be easy | 3. Hits placed anywhere |
| Layering should include a flam | 4. Stack or flam |
| Default drums should always come from the left panel | 5. No built-in sounds |
| Keeping a drum should not start a new litter (wanted two similar hats) | 6. Keeping leaves the litter |
| Kick stayed "locked" after unlocking | 7. Breeding that visibly changes unlocked lanes |
| Dragged-out MIDI on the same track still triggers the right panel | 8. Playing is an explicit switch |

## 1. None options

**Zone (left):** `[None][Kick][Snare][Hats][Perc]`.
- **None** breeds anywhere (as the old "Other" did). Pads kept from None are named "Drum 1", "Drum 2" and so on, and are **wildcards**: any lane may use them (see 5).
- **Perc** is a new zone, defined in this prototype only (do not change `plugins/bloodline-drums/core`; add the ranges in a small table in `core/drums.cpp` and apply them inside `DrumOps`). A first guess, to tune by ear: Pitch 200 to 900 Hz, Sweep 0 to 6 st, Sweep time 5 to 40 ms, Attack 0 to 2 ms, Decay 20 to 200 ms, Body / Noise 0.2 to 0.6, Noise tone 1.5 to 8 kHz, Noise decay 20 to 150 ms, Metal 0 to 0.3, Click 0.2 to 0.8, Click tone 1.5 to 6 kHz. Toms, rims, woodblocks, clicks.
- Labels map to lanes: Kick to kick, Snare to snare, Hats to hat, Perc to perc.

**Style (right):** `[None][Boom bap][Lo-fi][Strung][Neo soul]`.
- **None** uses the flat odds (the `kFlat` row), no lean, no extra scatter, no rolls. Choosing it does not move the swing dial. Pull then only pulls swing.

Both rows must still fit beside [Back][New] in a 616 px panel: five buttons of about 64 px after a 44 px label.

## 2. Spread the user can drag

- Both panels get a **spread slider**. Left: in the `[loop][spread]` row, replacing the read-only meter. Right: a fifth control in row 2, `[playing][midi][swing][pull][spread]` (shrink the swing and pull sliders to make room).
- Dragging calls `Litter::setSpread`. Breeding keeps its automatic rules (Again narrows, a new tile twice in a row widens) and the slider follows.
- **[New] resets the spread** to the litter's starting value. `Litter::refresh` already does this; add a test that proves it for both panels, and make sure the slider visibly jumps back.
- Show the value as a number beside the slider (two decimals).

## 3. Hits placed anywhere

The step grid becomes a guide, not a cage.

**Data:** a lane is a list of hits, not sixteen steps. Each hit has:
- `pos`: where it sits in the bar before swing, in sixteenths, a double from 0 to just under 16. Anywhere, not only on steps.
- `vel` (0.08 to 1), `roll` (1 to 4).
- At most 32 hits a lane. Two hits closer than 1/64 of a bar on one lane are one hit.

**Swing becomes a warp of time:** within each eighth (two sixteenths), position `x` from 0 to 2 lands at `2 * swing * x` for x below 1, and `2 * swing + (2 - 2 * swing) * (x - 1)` above. On the grid this is exactly today's swing; off the grid it bends smoothly. `hitsOf` applies it; the editor draws hits where they land.

**Breeding:** fresh draws still roll the odds on each of the sixteen steps, then add the style's nudge to `pos`. Mutation moves each hit by `gauss * spread * 0.6` sixteenths (folded back into the bar), may remove it, and may add hits on empty steps (see 7). Cross takes whole lanes as now.

**Editing in the right panel's settings (the big grid):**
- **Click empty space:** adds a hit at the nearest sixteenth, velocity 0.9.
- **Press on a hit and drag:** sideways moves it freely in time, no snap; up and down sets its velocity (drawn as height). A hit dragged past either end of the bar wraps round.
- **Shift while dragging:** snaps to sixteenths.
- **Click a hit without moving:** removes it.
- The step columns stay drawn faintly as a guide. Ghost notes are just low velocity.

The tiles and kept slots keep their small grids, drawn from the same data.

## 4. Stack or flam

Each lane of a loop has a **sound** of one or two kept pads, and a **layer** setting that matters only with two:
- **Stack:** both play together (as now).
- **Flam:** the first plays on the hit; the second follows `flam` ms later (8 to 40 ms) at `flamLevel` (0.4 to 0.8) of the hit's velocity.
- A flam may use **the same pad twice**, so a single snare can be flammed. Give each pad two voices, used in turn, so a flam never cuts its own first hit.

**Bred:** whether a lane layers (chances as now: kick 0.1, snare 0.35, hat 0.15, perc 0.2), then stack or flam (even odds; snare and perc favour flam 0.6), then `flam` and `flamLevel` uniformly in range. Mutation nudges `flam` and `flamLevel` by `gauss * spread`, and with chance `spread * 0.5` re-draws the layer choice.

**By hand, in each lane's head in the settings panel:** `[first sound | second sound][One/Stack/Flam][Lock]`.
- Click the first half: cycles the first sound through the lane's choices (as now).
- **Drag a kept pad from the left panel onto either half** to set that sound. This is how the user picks a layer by hand.
- The layer button cycles One, Stack, Flam. In Flam it shows the ms; drag sideways on it to change the ms.
- The second half shows `+` when empty and an `x` to clear it when full.

The dragged-out MIDI writes the flam's second note at its delay and level.

## 5. No built-in sounds

Every sound the right panel plays comes from the left panel's keep row.
- A lane chooses from, in order of preference: pads labelled for it; then Drum (None) pads; then any other kept pad, rarely (chance 0.15 a lane, 0.4 for perc).
- With nothing kept at all, loops still breed and draw, but are silent, and the right panel's settings show "Keep a drum on the left to hear the loops."
- A lane whose pads are all cleared plays nothing until the next breed re-chooses (mutation already re-draws a sound that plays nothing).
- Remove the built-in voices, `builtIn()`, and notes 44 to 47. The dragged-out MIDI uses only 36 to 43.

## 6. Keeping leaves the litter

- Dragging a drum tile onto a pad keeps it **and leaves the litter as it is**, so the same tile, or a sibling, can be kept on another pad. Remove the `refresh` from `Drums::keep`.
- Names still count up ("Hats 1", "Hats 2"), so two similar hats are told apart.
- Update the README and the test "keeping puts the candidate on the pad..." which checks the refresh.

## 7. Breeding that visibly changes unlocked lanes

**What was found:** locks work. The report "the kick was still locked after unlocking" is breeding being too gentle. With the kick unlocked, pressing Again (spread 0.12) gave three children whose kicks differed from the parent on **0 of 48** steps; two more Agains (spread 0.04): **1 of 48**. Steps are only re-rolled with chance `spread * 0.8`, and a re-roll usually lands where it was.

**Change:**
- Each unlocked lane of each child makes at least one real change: a hit added, removed or moved by more than half a sixteenth. On top of that, about `1 + spread * 8` changes a lane on average, chosen from add (on an empty step, weighted by the odds), remove (weighted by how unlikely the step is), and move.
- Spread still scales how far moves go and how many changes happen; the minimum of one keeps children distinct even at the lowest spread.
- Make locks easy to read: a locked lane's row in the settings and on every tile gets a faint lock-coloured band.

**Tests:** the scenario above (lock, breed, keep, unlock, Again) must give every child a kick that differs from the parent's; and a locked lane must stay identical through breed, cross and Again.

## 8. Playing is an explicit switch

- **[Playing] is off by default.** Off means the right panel makes no sound, whether Live is playing or not.
- **On:** while Live plays, the loop follows Live's bar; while Live is stopped, it runs on its own at Live's tempo from the top of the bar.
- **Dragging MIDI out turns Playing off**, so the clip on the track takes over. The button then reads "Play" and a short note under the Drag MIDI handle says "Playing off: the clip plays the kit".
- Choosing **In break** on the left turns Playing on.
- Incoming notes 36 to 43 always play the pads, whatever Playing is. Notes 48 to 55 still switch kept loops.

## Layout after the changes

```
LEFT                                         RIGHT
[ZONE None Kick Snare Hats Perc][Back][New]   [STYLE None Boom bap Lo-fi Strung Neo soul][Back][New]
[LOOP Off Quarters Boom bap 16ths In break]   [Playing][Drag MIDI][SWING ---][PULL ---][SPREAD ---]
[SPREAD ----- 0.25]  (same row, shrink the       [A][B][C][D]
 pattern buttons to fit)                       settings: four lanes, each [sound | sound][Flam][Lock],
[A][B][C][D]                                   then the free grid, then this loop's swing
settings: 19 settings, two columns            [KEEP 8 loops]
[KEEP 8 pads]
```

Keep the window at 1280 by 700 if it fits; grow the height rather than crowd it.

## Saved state, version 2

The text format stays line based. Changes: zone and style indices shift (None is first in both); each pattern line holds, per lane, the sound (two pad indices, -1 for none), layer (0 one, 1 stack, 2 flam), flam ms, flam level, the hit count, then `pos vel roll` per hit; `play` is saved as before.

## Tests to add (core, no JUCE)

1. None zone and None style: a None style litter has no lean (backbeat snares no likelier than other steps, over many seeds).
2. Spread: `setSpread` holds within limits; New resets both panels to the start.
3. Free hits: a hit at 5.3 sixteenths with swing 0.66 lands where the warp says; hits wrap at the bar's end; duplicates merge.
4. Flam: the player emits the second hit `flam` ms later at the right level; one pad flammed with itself plays both.
5. No built-ins: with nothing kept every lane chooses nothing; with only a Drum pad, every lane may use it.
6. Keep: the litter is unchanged after keeping; one tile kept twice gives two pads.
7. The lock scenario in 7.
8. Saving and loading version 2 gives back everything; version 1 is rejected.

## How to build and check

- Work on branch `claude/plugin-album-collection-5nvyi5`. Commit and push there.
- Core: `cmake -S plugins/bloodline-breaks-2 -B build-b2 -G Ninja -DBLOODLINE_BREAKS2_PLUGIN=OFF && cmake --build build-b2 && ctest --test-dir build-b2 --output-on-failure`.
- Plugin: the full build fetches JUCE 9.0.5. Check the layout by rendering the editor to a PNG under `xvfb-run` (a small scratch program that makes the processor, fills it with a few kept drums and loops, and calls `createComponentSnapshot`), and look at it before committing.
- CI ([workflow](../../.github/workflows/bloodline-breaks-2.yml)) runs the tests, pluginval at strictness 10 and the Mac VST3 and AU builds.
- Update [README.md](README.md): how to play it, the MIDI map (36 to 43 and 48 to 55 only), the checklist, and the known shortcuts.

## Out of scope

Taste learning; more than one bar; more than four lanes; triplet grids; anything in `plugins/bloodline-drums/core` or `plugins/shared`.
