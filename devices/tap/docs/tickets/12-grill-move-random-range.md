# 12 (grill): Move by a random amount within a range

**Status:** to grill. Depends on ticket 11 (what Move's control becomes).

**Idea (the user, 2026-10-25):** they mapped an LFO to Move so it drifted slowly between -2 and +2 semitones. It kept things random without the keys shifting off the keyboard too quickly. A random amount within a range would be nice.

## Where things stand

- Move adds a fixed amount (currently Mod) to the released key on every release.
- Mod is a `live.dial`, so a Live LFO can already be mapped to it. That is the setup the user liked.
- Ticket 09 (Wander) planned "current moves by a random amount up to Mod either way". This ticket would replace it.
- An LFO that drifts slowly gives **neighbouring releases similar amounts**. A fresh random number per release does not. The two feel different.

## Decided with the user (2026-10-26, round 1)

- **What they liked about the LFO:** the range was balanced. It went back and forth and landed anywhere within it, say one or two semitones up or down, with every amount equally likely. The same should work for Gravity.
- **Not a mode:** Random is a modifier on every mode, not a new mode and not only part of Move.
- **Its own knob:** Mod stays the normal setting, wherever it is set. A separate Random knob sets how far each release can land from it.
- **Any amount,** not whole semitones. No snap switch for now.
- **Not repeatable:** no seed.

## Questions to grill

1. **Which part of the LFO setup did they like?** The range staying within ±2, the slow change between releases, or both? *Recommendation:* ask the user to play the LFO setup again and say which. It decides question 2.
2. **What sort of random?**
   - **A. Fresh each release:** a new amount between Min and Max on every release.
   - **B. Random walk:** each release's amount is the last one plus a small random step, kept within the range. This is closest to a slow LFO.
   - **C. Built-in LFO:** a sine or triangle sampled on each release, with its own rate.

   *Recommendation:* B. It keeps the slow, related feel without tying it to tempo or time.
3. **How is the range set?** Two controls, Min and Max, or Centre and Spread? *Recommendation:* Min and Max (defaults -2 and +2). They read most clearly on the panel and match how the user described it.
4. **Whole semitones or any amount?** *Recommendation:* any amount by default, with a "Snap to semitones" switch. In Semitone output the result is rounded anyway.
5. **Is this a new mode, or an option within Move?** For example, a Move amount switch: Fixed or Random. *Recommendation:* an option within Move. It keeps the menu at three (ticket 11), and Fixed is Random with Min equal to Max.
6. **Random walk step size (if B):** fixed, or its own control (Speed or Drift rate)? *Recommendation:* its own control, so the user can choose between slow drift and jumpy.
7. **Each key on its own walk, or one walk shared by every key?** One shared walk is like the LFO they used. *Recommendation:* one shared walk, to start.
8. **Repeatable?** Should the same playing give the same result each time (a seed), or not? *Recommendation:* not. TAP is meant to be unpredictable.
9. **Should Gravity or Swap get randomness too,** or is this for Move only? *Recommendation:* Move only for now. Ticket 13 adds chance to every mode.

## Done when grilled

- Answers recorded here.
- Ticket 09 marked as replaced by this one.
- Rewritten as a build ticket.
