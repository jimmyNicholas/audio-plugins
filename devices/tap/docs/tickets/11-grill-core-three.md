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

## Decided with the user (2026-10-25, round 1)

- **Why:** it was getting too complex too fast. Things should move out slowly. Some new modes overlap with the old ones. The new panel UI (ticket 14) should be tried with three modes first, so there are fewer moving parts. The other modes come back later.
- **New modes:** take Repel, Echo, Rotate, Wander and Harmony out of the menu. Tickets 07 to 10 stay as paused tickets, so the ideas are not lost.
- **Feel per mode:** not needed for this ticket.
- **Gravity stays in Hertz,** as in the web app. It should be more finely tuned, so it is less severe. That tuning can be part of the UI work.
- **Swap:** unchanged. It is the simplest mode, and doing nothing on the first release is right.
- **Lowest and Highest:** keep them. A double (range) slider for setting both would be more useful.
- **Mode change resets every key:** keep it.
- **Old saved sets:** do not matter.
- **Tickets 06 and 07:** skip testing them in Live. Only the reworked device gets tested.
- **Later, out of scope:** a floor at the same note name. If you play A4 and Gravity pulls it down, it stops at the A below instead of going on down. It needs its own ticket.

## Decided with the user (2026-10-25, round 2)

- **One shared Mod dial for every mode,** reading -12 to +12 in semitones. Gravity turns the Mod value into its own strength.
- **Gravity is far too strong.** It needs bringing down a lot: Mod 12 should pull about 50% of the gap, Mod 1 about 1% and Mod 2 about 3%, rising like an exponential in between.
- **Mode control on the Live strip:** a slider for now.
- **Double slider for Lowest and Highest:** its own ticket ([16](16-range-slider.md)), blocked by tickets 11 to 15.
- **Order of work:** grill every ticket (11 to 15) first, then build.
- **Floor at the same note name:** parked as ticket [17](17-note-floor.md).

## Decided with the user (2026-10-25, round 3)

- **Gravity's curve:** `share = 1% × Mod^1.585`, so each doubling of Mod triples the pull: Mod 1 is 1%, 2 is 3%, 4 is 9%, 8 is 27% and 12 is 51%. It stays in Hertz, never overshoots and never lands one key fully on another. The curve's numbers can be changed in a debug setting.
- **Gravity with a negative Mod:** pushes the last key away from the current one, on the same curve.
- **Swap reads Mod:** at +1 it is the normal Swap. From +2 to +12 it rotates pitches through the last keys pressed (this brings ticket 08's Rotate into Swap). A negative Mod turns the rotation the other way.
- **Move:** unchanged. It adds Mod in semitones, -12 to +12, in 0.01 steps.
- **Mod's default:** stays at 1.
- **Mode on the Live strip:** a three-step slider: Swap, Gravity, Move.

## Decided with the user (2026-10-25, rounds 4 to 6)

- **Swap's count:** Mod rounded to a whole number, ignoring the sign, plus 1. Mod +1 swaps 2 keys, +2 rotates 3, and so on up to +12, which rotates 13. A Mod that rounds to 0 changes nothing.
- **Fewer releases than the count:** rotate what's there. With 2 releases it swaps, with 3 it rotates 3, and so on up to the count.
- **Turning Mod keeps the list.** TAP always remembers the last 13 releases and rotates the newest N, so an LFO on Mod changes the size of the circle smoothly.
- **Direction:** with a positive Mod, each place takes the pitch of the place released after it, and the newest takes the oldest's. Release C, E, G at Mod +2: C plays E, E plays G, G plays C. A negative Mod turns it the other way: C plays G, E plays C, G plays E.
- **Repeats:** the list holds releases, not keys, so a key released twice holds two places. When one rotation gives a key two pitches, its **newest place wins**. Example: Mod +2, release C, E, C: C and E both end up playing C, so pitches can merge over time.
- **Pedal:** each queued release joins the list and rotates in turn when the pedal lifts, as in every mode.
- **Gravity's debug settings** (1% at Mod 1, ×3 per doubling): fixed in code for ticket 11. Ticket 14 adds them to a debug section in the panel window. They are not Live parameters and are not saved with the set. Values the user likes become the new defaults in code.

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
