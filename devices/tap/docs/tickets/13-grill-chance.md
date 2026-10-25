# 13 (grill): Chance to fire

**Status:** to grill. Name not chosen yet. Depends on ticket 11.

**Idea (the user, 2026-10-25):** a dry / wet control for the "chance to fire" of a mode (better name needed).

## Where things stand

- Every release applies the mode. Nothing is left to chance.
- In music software, "dry / wet" usually means a blend, not a probability. The user's words may mean either of two different controls:
  - **Chance:** at 30%, about three releases in ten change pitch fully and the rest change nothing.
  - **Amount:** at 30%, every release changes pitch, but only 30% of the way. Swap would move each key 30% of the way toward the other's pitch.

## Questions to grill

1. **Chance, Amount, or both?** *Recommendation:* Chance first, since that is what "chance to fire" says. Amount could come later as its own control.
2. **Name.** Candidates: **Chance** (Ableton's own word for this, in its MIDI effects and clip notes), Probability, Odds, Mischief, Annoyance. *Recommendation:* Chance, because Live users already know what it does. Annoyance is the fun option if the user wants character.
3. **Range and default:** 0 to 100%, default 100% (every release fires, as now)? *Recommendation:* yes.
4. **One control for every mode, or per mode?** *Recommendation:* one control for every mode. It shows on the panel next to the mode controls.
5. **When a release does not fire, does it still count as the last key released?** This matters for Swap and Gravity, which pair the current key with the last one.
   - Counts: the next release pairs with this key, even though it did not change.
   - Does not count: the next release pairs with the key before it, so the pairs stay the same as at 100%, with some skipped.

   *Recommendation:* counts. Then a release that does not fire behaves exactly as if the mode had nothing to do that time.
6. **Sustain pedal:** each queued release gets its own roll when the pedal lifts? *Recommendation:* yes, one roll per release.
7. **Should the drift view show misses,** for example by flashing the key? *Recommendation:* no, not at first.
8. **Can a Live LFO or automation drive it?** *Recommendation:* yes. Make it a `live.dial` parameter like the others, so it saves with the set and can be automated.

## Done when grilled

- Answers recorded here, including the name.
- Rewritten as a build ticket.
