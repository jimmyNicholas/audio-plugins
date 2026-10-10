# 05: Drift view

**Goal:** see how far each key has drifted from concert pitch.

## Tasks
- Decide the view first, with the user: a wider panel in the device, or a pop-out window.
- The `js` sends the pitch table to the view after each change, not on every note.
- Show each key's offset in cents, with colour for sharp and flat.

## Prototype
Play and watch keys drift as they are released. Reset clears the view.

## Done when
- The view updates without slowing note handling (check with the same fast passages as ticket 01).
