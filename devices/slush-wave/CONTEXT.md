# Slush Wave

A Max for Live audio effect that gives a track the slushwave sound. This glossary covers the words the spec, the panel and the tickets share.

## Slowing

**Slowing**:
Varispeed playback of the source, where pitch and tempo fall together, like a turntable running slow. Slush Wave never slows; it happens outside the device, before or after it.
_Avoid_: Time stretch, slowdown stage

**Pre-slowing**:
The workflow where the source is slowed first (Re-Pitch warp or a bounce) and Slush Wave runs on the slowed audio.

**Post-slowing**:
The workflow where Slush Wave runs on the original, the result is bounced, and the bounce is slowed, so the effects are slowed too.

## Stages

**Pitch layer**:
Pitch-shifted copies of the input mixed back in to make the sound fuller. Made of detuned doubles and an interval layer, each with its own level. It thickens the sound and is not slowing.
_Avoid_: Pitch down, slowdown

**Detuned doubles**:
Copies a few cents above and below the input, spread left and right.
_Avoid_: Chorus (a separate, modulated stage)

**Interval layer**:
A copy shifted by a musical interval (an octave or a fifth down) mixed under the input for weight.
_Avoid_: Sub, octaver
