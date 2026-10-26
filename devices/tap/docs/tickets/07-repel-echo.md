# 07: Gravity on Mod, Repel and Echo

**Status:** paused (2026-10-25). Ticket 11 takes it out of the device. The idea comes back after the core three and the panel.

**Goal:** the first two new modes, with Gravity and Repel both reading the Mod knob.

## Decided with the user (2026-10-25)
- Mod's positive and negative range is for strength and repel. **Gravity reads Mod** instead of the Strength dial, and the Strength dial goes.

## The modes
- **Gravity:** Mod is the web app's strength. A positive Mod pulls last toward current; a negative Mod pushes it away.
- **Repel:** Gravity with the sign turned round. A positive Mod pushes last away from current; a negative Mod pulls.
- **How far:** for a strength `s` (Mod, ignoring the sign), the share of the gap moved, in Hertz, is:
  - `s` from 1 to 10: `1 / (10 - s)`, the web formula. Mod 2 is the web app's default.
  - `s` below 1: `s / 9`, fading in from nothing at Mod 0 and meeting the web formula at 1.
  - `s` of 10 or more: a full pull lands exactly on current; a full push goes to a bound (Highest if last is above current, Lowest if below).
- Results past a bound are held at the bound. If last and current already play the same pitch there is no direction, so nothing moves. On the first release, or the same key twice, nothing moves.
- **Echo:** current takes last's drift, plus Mod: `drift(current) = drift(last) + mod`. Mod 0.10 adds 10 cents per key, so a phrase leaves a trail of drift that grows along it. On the very first release last is current, so the key just moves by Mod.

## Tasks
- `tap.js`, test-first: `REPEL = 3` and `ECHO = 4`. Gravity and Repel share one function, with Repel passing `-mod`. Remove `setStrength`, `strength()` and the `strength` Max handler, and `strength` from `sync`.
- Tests: the web Gravity values with Mod as strength, a negative Mod pushing, Mod 0, Mod below 1, Mod above 10, Repel matching Gravity with the sign turned, full pushes to each bound, no direction at one pitch, Echo's trail up and down, and the bounds.
- Patch: remove the Strength dial and its `prepend`; Mod takes its place. Regenerate the device.

## Prototype
- **Gravity**, Mod 2.00: as the web app. Mod -2.00: keys push apart instead.
- **Repel**, Mod 5.00: play A3 then C4 a few times, releasing each. A3 sinks and C4 rises until they reach the bounds. The drift view shows them moving apart.
- **Echo**, Mod 0.10: play a rising scale once. Each key is 10 cents sharper than the one before. Play it again and the trail carries on from where it ended.

## Done when
- Tests pass, including the ES5 check.
