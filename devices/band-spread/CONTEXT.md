# Band Spread

A Max for Live audio effect that cuts one narrow band out of the input and plays copies of it at other places in the spectrum at once. This glossary covers the words the map, the tickets and the panel share.

## Language

**Band Spread**:
The device: one band in, several copies of it out, spread across the spectrum.
_Avoid_: The plugin, the spreader

**Band**:
The narrow slice of the input that Band Spread keeps, set by a centre frequency and a width. Everything outside it is removed; Band Spread has no dry signal.
_Avoid_: Slice, filter, region

**Copy**:
One shifted instance of the band, with its own on/off, level, shift, pan and mono. All copies sound at the same time.
_Avoid_: Voice, layer (Slush Wave's word), harmonic, partial

**Shift**:
How far one copy moves from the band's own position. Whether it is a pitch shift (keeps harmonic ratios) or a frequency shift (adds a fixed number of Hz) is still open.
_Avoid_: Transpose, offset, detune

**Spread**:
The effect of all copies sounding together, so one narrow band covers much more of the spectrum. Not a sweep: nothing moves over time.
_Avoid_: Sweep, smear, stretch

## Relationships

- Band Spread has exactly one **Band**.
- A **Band** feeds every **Copy**; each **Copy** has its own **Shift**.
- The **Spread** is the sum of the copies that are on.

## Flagged ambiguities

- "Pitches the band across the spectrum" could mean a sweep over time or copies at once. Resolved: copies at once (**Spread**), static for now.
