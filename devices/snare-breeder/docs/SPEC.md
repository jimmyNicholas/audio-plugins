# Spec: Snare Breeder

Working name. The snare version of [Kick Breeder](../../kick-breeder/docs/SPEC.md): a Max for Live instrument for Ableton Live 11 that synthesises a snare with no samples. You find the snare you want by choosing between candidates, round after round, and keep it.

Status: idea. A throwaway spike is in [`spike/`](../spike/README.md), with a checklist to fill in. Decisions marked **(proposed)** are the builder's recommendation and still need the user's yes.

## The same as Kick Breeder

The loop, breeding, fair listening, listening in context, keys and keeping all follow the Kick Breeder spec. The code is shared, in [`shared/breeder/`](../../../shared/breeder/README.md), so a change to the loop reaches both devices.

## One device per drum (proposed)

Kick, snare and (later) hats are separate devices on one shared engine, not one device with a drum switch:

- Each patch holds only its own sound engine, so it stays simple.
- Each drum's settings have their own names and ranges, so automation and presets make sense.
- Drum Rack has one instrument per pad, so one drum per device fits it.
- Each drum needs its own tuning: random ranges, loudness weighting, and (for hats) closed and open notes that choke.

The cost is a patch per drum to look after; the shared builder keeps that small.

## The snare voice

A snare is two sounds at once: the drum skin and the rattle of the snare wires under it. Built from plain MSP objects (no `gen~`):

| Part | Settings | Range (proposed) |
| --- | --- | --- |
| Skin | Pitch; a second tone at Ratio times Pitch, Ring loud; one pitch sweep for both; Body decay | 120 to 400 Hz; 1.3 to 2.4, 0 to 1; 0 to 24 semitones over 5 to 80 ms; 30 to 500 ms |
| Balance | Skin to wires, equal power | 0 to 1 |
| Wires | Band-passed noise: decay, tone, width | 40 to 800 ms, 800 Hz to 10 kHz, wide to narrow |
| Bursts | The wires hit 1 to 4 times, Spread apart, before the tail. Half of all random snares have 1 | 1 to 4, 4 to 20 ms |
| Snap | A short crack of high-passed noise | 0 to 1 |
| Comp, Sat, Tone | As on the kick | 0 to 1, 0 to 1, 1 to 16 kHz |

Sixteen settings, against the kick's thirteen. Bursts means one device covers claps too, which may or may not be wanted (open question 2).

## Fair listening: weighted loudness (proposed)

Snares carry much more of their energy at 2 to 5 kHz than kicks, where ears are most sensitive. Plain RMS matching would make bright snares sound louder, and a louder sound nearly always wins an A/B. So the snare's loudness simulation lifts everything above about 1.5 kHz by 4 dB before measuring: a rough stand-in for K-weighting, as in LUFS meters.

## Open questions

1. **Does a plain-MSP snare sound good enough?** The spike's first job.
2. **Claps in the snare device, or their own?** Bursts makes claps reachable; it may also pull searches away from snares.
3. **Are sixteen settings too many** to breed in about ten rounds?
4. **Drum Rack:** a Drum Rack sends C3 to every pad's instrument by default, and C3 plays slot A. Should the play keys move, be switchable, or should there be a "play mode" where every note plays the kept drum?
5. **Name:** Snare Breeder is a placeholder.
