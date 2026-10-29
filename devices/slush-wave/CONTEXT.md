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

## Paths

**Centre path**:
The copy of the input that carries the low end: highs cut, compressed, lightly distorted, and kept dry of reverb.
_Avoid_: Bass channel, mid

**Sides path**:
The copy of the input that carries the wet sound, its left and right sides run through the same stages, except the phaser, which sweeps the opposite way on each side.
_Avoid_: Wet chain, side (as in mid/side)

## Stages

**Stage**:
One effect in a path, such as a phaser or a reverb. A path can hold any stages in any order, and the same kind more than once.
_Avoid_: Module, block, slot

**Barberpole phaser**:
A subtle, low feedback phaser whose sweep seems to rise or fall forever, rising on the left and falling on the right.
_Avoid_: LFO phaser

**Pitch layer**:
Pitch-shifted copies of the input mixed back in to make the sound fuller. Made of detuned doubles and an interval layer, each with its own level. It thickens the sound and is not slowing.
_Avoid_: Pitch down, slowdown

**Detuned doubles**:
Copies a few cents above and below the input, spread left and right.
_Avoid_: Chorus (a separate, modulated stage)

**Interval layer**:
A copy shifted by a musical interval (an octave or a fifth down) mixed under the input for weight.
_Avoid_: Sub, octaver
