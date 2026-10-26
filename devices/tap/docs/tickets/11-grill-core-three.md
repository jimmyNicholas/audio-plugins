# 11: Back to the core three

**Status:** grilled 2026-10-25. Ready to build, but the user wants tickets 12 to 15 grilled before any code.

**Goal:** three modes only: Swap, Gravity and Move, each reading the one Mod dial in its own way. Fewer moving parts before the panel window (ticket 14).

## Why

It was getting too complex too fast. Some new modes overlap with the old ones. The new panel should be tried with three modes first, and the others come back later and slowly. Tickets 07, 09 and 10 stay as paused tickets. Ticket 08 (Rotate) becomes part of Swap.

## The modes

Mod is one dial, -12 to +12 in semitones, in 0.01 steps, with a default of 1.

### Swap

- **Count:** Swap passes pitches round the newest N releases, where N is Mod rounded to a whole number, ignoring the sign, plus 1.
  - Mod +1 swaps 2 keys, the normal Swap.
  - Mod +2 rotates 3 keys, and so on up to Mod +12, which rotates 13.
  - A Mod that rounds to 0 changes nothing.
- **The list:** TAP always keeps the last 13 releases.
  - Turning Mod does not clear the list, so an LFO on Mod changes the size of the circle smoothly.
  - With fewer releases than N, it rotates the ones it has. The very first release, or the same key twice in a row, changes nothing, as before.
- **Direction:**
  - With a positive Mod, each place takes the pitch of the place released after it, and the newest takes the oldest's. Release C, E, G at Mod +2, and C plays E, E plays G, G plays C.
  - A negative Mod turns the other way: C plays G, E plays C, G plays E.
- **Repeats:** the list holds releases, not keys, so a key released twice holds two places. If one rotation gives a key two pitches, its newest place wins. For example, at Mod +2, release C, E, then C: C and E both end up playing C.

### Gravity

- Gravity still works in Hertz.
- **Strength:** the last key moves `share × gap` towards the current key, where `share = 1% × |Mod|^log2(3)`. Each doubling of Mod triples the pull:

  | Mod | 0.5 | 1 | 2 | 4 | 8 | 12 |
  | --- | --- | --- | --- | --- | --- | --- |
  | Share of gap | 0.3% | 1% | 3% | 9% | 27% | 51% |

- **Negative Mod:** pushes the last key away from the current one, on the same curve.
- **Never lands on the key:** no setting lands one key fully on another, and a pull never overshoots.
- **Below 0 Hz:** a push that would go below 0 Hz goes to Lowest.
- **Tuning:** the 1% and the ×3 are constants in code for now. Ticket 14 adds them to a debug section in the panel window. They are not Live parameters and are not saved with the set.

### Move

Unchanged: the released key moves by Mod semitones.

### All modes

- Lowest and Highest bound every result.
- Changing mode or pressing Reset puts every key back to concert pitch and clears the list of releases.
- The sustain pedal queues releases. Each one is applied in turn when the pedal lifts.

## Tasks

- `tap.js`, test-first:
  - Take out Repel, Echo, Rotate, Wander and Harmony: their constants, rules and tests. `setMode` clamps to 0 to 2.
  - Swap: a list of the last 13 releases, plus the rotation above.
  - Gravity: the new `share` curve, with its two constants (`GRAVITY_AT_ONE = 0.01` and `GRAVITY_PER_DOUBLING = 3`) named in one place for ticket 14.
- Tests:
  - **Swap:**
    - Mod +1 matches the old Swap, release for release.
    - Mod +2 rotates C, E, G both ways.
    - It rotates fewer keys than N while the list fills.
    - Mod rounding to 0 changes nothing.
    - Turning Mod keeps the list.
    - A repeated key's newest place wins.
    - The pedal applies releases one by one.
    - Reset and a mode change clear the list.
  - **Gravity:**
    - The share table above.
    - A negative Mod pushes.
    - Mod 0 changes nothing.
    - A push past 0 Hz goes to Lowest.
    - Keys never land exactly on the current key.
  - The ES5 check still passes.
- Patch:
  - Replace the Mode menu with a three-step `live.slider` (Swap, Gravity, Move), keeping the `mode` varname and the parameter name `Mode`. It shows the mode's name, not a number.
  - Update the patch tests: the slider has three steps in engine order, and every control is still wired through its `prepend`.
  - Regenerate `TAP.amxd`.

## Prototype

The patch changes, so drag the device in again.

1. `git pull`, then `npm test --prefix m4l`. The last lines show `# fail 0`.
2. **Mode slider:** it has three steps, showing Swap, Gravity and Move. Changing it resets the drift view.
3. **Swap, Mod +1:** release A, then B. Play A: it sounds as B.
4. **Swap, Mod +2:** release C, E, G. Play C, E, G: they sound as E, G, C. At Mod -2 they sound as G, C, E.
5. **Swap with an LFO on Mod:** play a phrase. The circle grows and shrinks with no stuck or silent notes.
6. **Gravity, Mod 1:** play two keys an octave apart back and forth. They move together very slowly; in the drift view, about 1% of the gap per release.
7. **Gravity, Mod 12:** each release closes about half the gap. Mod -12 pushes them apart until they reach Lowest and Highest.
8. **Move:** as before.

## Done when

- Tests pass, including the ES5 check and the patch checks.
- The user has played all three modes in Live.
