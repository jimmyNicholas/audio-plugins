# 06: Repel and Echo

**Goal:** the first two new modes, and a Mode control with room for eight.

Part of "three modes to eight" (tickets 06 to 10). Rules for all the new modes: [`../SPEC.md`](../SPEC.md#more-modes).

## The modes
- **Repel:** Gravity in reverse. Last is pushed away from current by `gap / (11 - strength)`, in semitones, so Strength 2 pushes about a ninth of the gap and Strength 10 doubles the distance. Uses the existing Strength dial.
- **Echo:** current takes on last's drift, plus a Nudge: `drift(current) = drift(last) + nudge`. Played phrases leave a trail of drift that grows along the phrase. On the very first release last is current, so the key just gets the nudge, as Move acts on the first release.

## Check with the user first
- **Repel formula.** Gravity's web formula cannot be reversed as it is: at strength 10 it would push forever. The formula above is finite and mirrors Gravity's dial. It works in semitones, not Hertz, so it sounds the same in every register.
- **Echo needs a nudge.** As first described ("current takes on part of last's drift"), Echo does nothing from concert pitch, because every drift starts at 0. The Nudge (cents, -50 to 50, default 10) gives it something to spread.
- **Mode control.** Eight labels do not fit the 150 pixel `live.tab`. Choose a two-row `live.tab` (4 by 2, everything visible) or a `live.menu` (a dropdown, smallest).

## Tasks
- `tap.js`, test-first in `tests/tap.test.js`:
  - `REPEL = 3` and `ECHO = 4`. New modes are added at the end, so sets saved with Swap, Gravity or Move keep their mode.
  - Repel, with the result kept within 0 to 127. Echo, with `setNudge` and the result kept within 0 to 127.
  - `setMode` accepts 0 to 7, so tickets 07 to 09 only add rules. Modes not yet built fall back to Swap, as now.
- Panel:
  - The Mode control with all eight names (Swap, Gravity, Move, Repel, Echo, Rotate, Wander, Harmony).
  - A `live.numbox` Nudge, saved with the set and automatable, read by `sync`.
- Regenerate the device. You will need to drag it in again.

## Prototype
- **Repel**, Strength 5: play A3 then C4 a few times, releasing each. A3 sinks and C4 rises; the drift view shows them moving apart.
- **Echo**, Nudge 10: play a rising scale once. Each key is 10 cents sharper than the one before. Play it again and the trail carries on from where it ended.
- Rotate, Wander and Harmony are in the list but act as Swap until their tickets.

## Done when
- Tests pass, including the ES5 check.
- Mode and Nudge survive saving and reopening the set, and a set saved before this ticket opens on the same mode.
