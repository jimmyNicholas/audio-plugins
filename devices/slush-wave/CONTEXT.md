# Slush Wave

A family of Max for Live stage devices and an Audio Effect Rack preset that give a track the slushwave sound. This glossary covers the words the spec, the panels and the tickets share.

## The family

**Slush Wave**:
The whole family: the Slush devices plus the `Slush Wave` rack preset that builds the default paths from them and stock Live devices.
_Avoid_: The plugin, the device (there is no single device)

**Slush device**:
One of the family's own Max for Live stage devices, named with the `Slush` prefix, such as Slush Phaser. Built only for stages Live's stock devices cannot do.
_Avoid_: Plugin, module

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

**Mix**:
A path's output setting where its sound goes into the track's own output.

**Send**:
A path's output setting where its sound is silent in the track and picked up by another track, which processes or mixes it separately.
_Avoid_: Return, aux (those are Live's send/return tracks)

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
