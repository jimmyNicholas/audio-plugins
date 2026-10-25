# 11 (grill): Back to the core three

**Status:** to grill. No code until the questions below are answered and this is rewritten as a build ticket.

**Idea (the user, 2026-10-25):** pause and see the modes clearly. Not confident about how the new modes were added. Go back to Swap, Gravity and Move and implement them well.

## Where things stand

- The Mode menu lists eight modes (ticket 06). Repel and Echo work (ticket 07). Rotate, Wander and Harmony do nothing yet (tickets 08 to 10).
- Tickets 06 and 07 are built but **not yet tested in Live**.
- Swap reads nothing. Gravity reads Mod as its strength, and a negative Mod pushes instead of pulls. Move adds Mod, a float from -12 to +12.
- Lowest and Highest bound every mode's result.
- The web app had a separate control per mode: Strength 1 to 10 (default 2) for Gravity, Semitones -2 to 2 in whole steps (default 1) for Move.
- Known rough edges in the core three:
  - **Gravity just below 10:** the web formula `1 / (10 - s)` overshoots a long way. At 9.9 it moves ten times the gap, so the result is held at a bound.
  - **Gravity works in Hertz,** so a pull upward and a pull downward of the same interval move by different amounts in semitones.
  - **Move** walks off the keyboard quickly at large Mod values. The user found an LFO between -2 and +2 felt better (see ticket 12).
  - **Swap** does nothing on the first release, because last equals current.
- Saved sets store Mode as a number. Taking modes out of the menu, or reordering them, changes which mode an old set opens on.

## Questions to grill

1. **What happens to the five new modes?** Options: delete them from `tap.js` and the menu; hide them from the menu but keep the code; or keep them in the menu and park only 08 to 10. *Recommendation:* take all five out of the menu and the engine, and keep them in git history (tickets 07 to 10 stay as notes). It keeps the device and the tests small while the core three are reworked.
2. **What is wrong with how they were added?** Too many modes, the shared Mod knob meaning different things per mode, the menu, or the modes themselves? *Recommendation:* answer this before the rest, because it decides whether the shared Mod knob survives.
3. **One shared Mod knob, or a control per mode** (as in the web app)? *Recommendation:* a control per mode, with the panel showing only the selected mode's controls (ties into ticket 14). Each control can then have its own name, range and unit.
4. **Gravity's control:** go back to Strength 1 to 10 (web) or keep the -12 to +12 range with negative pushing? *Recommendation:* Strength 0 to 10, no push. Pushing was the Repel idea, which question 1 parks.
5. **Gravity's curve:** keep the web formula with its overshoot below 10, or smooth it so the share of the gap moved rises evenly from 0 at strength 0 to 1 (a full pull) at 10? *Recommendation:* smooth it. The overshoot is a divide-by-zero side effect, not a musical choice.
6. **Gravity in Hertz or in semitones?** *Recommendation:* semitones, so a pull feels the same in every register. Ask whether matching the web app exactly matters more.
7. **Move's control:** whole semitones -2 to +2 (web), or fractions? *Recommendation:* settle this together with ticket 12, since the random range may replace the fixed amount.
8. **Swap on the first release:** keep doing nothing? *Recommendation:* keep it. It matches the web app and nothing obvious is better.
9. **Lowest and Highest:** keep them for all modes? *Recommendation:* keep them. They are what stops Move and Gravity running off the keyboard.
10. **Mode change resets every key:** keep it? *Recommendation:* keep it (web behaviour), with Reset still separate.
11. **Old sets:** does it matter if sets saved with the eight-mode device open on the wrong mode? *Recommendation:* no. The device is only for the user so far.
12. **What does "implemented well" mean?** Write down two or three things the user wants to feel when playing each of the three modes. These become the Prototype steps of the build ticket.

## Done when grilled

- Each question has an answer recorded here.
- Tickets 08 to 10 are marked parked or removed.
- This ticket is rewritten as a build ticket with Tasks, Prototype and Done when, in the style of tickets 02 to 07.
