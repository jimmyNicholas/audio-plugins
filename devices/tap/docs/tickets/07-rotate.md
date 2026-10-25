# 07: Rotate

**Goal:** Swap for more than two keys.

## The mode
- TAP remembers the last N keys released, in release order. On each release the current key joins the list (the oldest drops off past N) and the keys pass their pitches round in a circle: each key takes the pitch of the key released after it, and the current key takes the pitch of the oldest.
- **Length** N runs 2 to 5, default 3. At 2 this is exactly Swap.
- A key already in the list moves to the end instead of appearing twice. If it was already the newest, nothing happens, as when Swap releases the same key twice.
- Nothing rotates until the list holds N keys. So the very first release does nothing, as in Swap.
- Reset, a mode change and a change of Length clear the list. (Length could trim the list instead; ask the user if clearing feels wrong.)

## Tasks
- `tap.js`, test-first: `ROTATE = 5`, `setLength`, the list and the rotation. Tests include Length 2 matching Swap release for release, a repeated key, and the pedal queue (keys rotate in release order when the pedal lifts).
- Panel: a `live.numbox` Length, 2 to 5, saved with the set and automatable, read by `sync`.
- Regenerate the device. You will need to drag it in again.

## Prototype
- **Rotate**, Length 3: play C, E, G, releasing each. Play C again: it sounds as E. E sounds as G, G sounds as C.
- In the drift view's Picture mode, squares move round in threes.

## Done when
- Tests pass, including the ES5 check.
- Length survives saving and reopening the set.
