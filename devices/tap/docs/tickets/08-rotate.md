# 08: Rotate

**Goal:** Swap for more than two keys.

## The mode
- TAP remembers the last N keys released, in release order. On each release the current key joins the list (the oldest drops off past N) and the keys pass their pitches round in a circle: each key takes the pitch of the key released after it, and the current key takes the pitch of the oldest.
- **Mod sets N and the direction.** N is Mod rounded to a whole number, ignoring the sign, from 2 to 12; anything below 2 counts as 2. A negative Mod turns the circle the other way: each key takes the pitch of the key released before it.
- At N = 2 this is exactly Swap, in either direction.
- A key already in the list moves to the end instead of appearing twice. If it was already the newest, nothing happens, as when Swap releases the same key twice.
- Nothing rotates until the list holds N keys. So the very first release does nothing, as in Swap.
- Reset, a mode change and a change of N clear the list.

## Tasks
- `tap.js`, test-first: `ROTATE = 5`, the list and the rotation. Tests include N = 2 matching Swap release for release, both directions, a repeated key, Mod below 2 and above 12, and the pedal queue (keys rotate in release order when the pedal lifts).
- No patch change.

## Prototype
- **Rotate**, Mod 3.00: play C, E, G, releasing each. Play C again: it sounds as E. E sounds as G, G sounds as C.
- Mod -3.00: the same keys go round the other way. C sounds as G.
- In the drift view's Picture mode, squares move round in threes.

## Done when
- Tests pass, including the ES5 check.
