# 04: Semitone mode

**Goal:** TAP works with instruments that do not support MPE, such as Operator.

## Tasks
- An Output choice on the panel: MPE or Semitone.
- In Semitone mode, send plain MIDI on channel 1 with each pitch rounded to the nearest semitone. Swap and Move stay exact; Gravity is rounded.
- Handle two held keys that round to the same note (for example, send a note off only when the last of them is released).
- Tests for rounding and the shared-note case.

## Prototype
With Operator after TAP and Output on Semitone, Swap and Move sound as expected and Gravity moves in whole semitones. No stuck notes.

## Done when
- Tests pass and the check above works in Live.
