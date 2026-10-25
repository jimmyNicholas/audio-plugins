# 07: Repel and Echo

**Goal:** the first two new modes.

## The modes
- **Repel:** the opposite of Gravity. Gravity's web formula with the sign turned round, in Hertz as Gravity is: `last = last + (last - current) / (10 - strength)`. Uses the Strength dial.
  - At strength 10, where Gravity pulls last exactly onto current, Repel pushes last all the way to a bound: Highest if last is above current, Lowest if it is below.
  - Results past a bound are held at the bound, as with every mode.
  - If last and current already play the same pitch, there is no direction to push in, so nothing moves.
- **Echo:** current takes last's drift, plus Mod: `drift(current) = drift(last) + mod`. Mod 0.10 adds 10 cents per key, so a phrase leaves a trail of drift that grows along it. On the very first release last is current, so the key just moves by Mod.

## Tasks
- `tap.js`, test-first: `REPEL = 3` and `ECHO = 4`.
  - Repel tests: a push matches the formula, strength 10 goes to each bound, last and current the same key or the same pitch, and pushes stopping at the bounds.
  - Echo tests: a rising phrase, a negative Mod, and the trail carrying on from the last key of the phrase before.
- No patch change: Mode already lists both.

## Prototype
- **Repel**, Strength 5: play A3 then C4 a few times, releasing each. A3 sinks and C4 rises until they reach the bounds. The drift view shows them moving apart.
- **Echo**, Mod 0.10: play a rising scale once. Each key is 10 cents sharper than the one before. Play it again and the trail carries on from where it ended.

## Done when
- Tests pass, including the ES5 check.
