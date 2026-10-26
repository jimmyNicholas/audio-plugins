# 15 (grill): Square visualiser

**Status:** to grill. Can be grilled on its own; it ends up as the last section of the panel window (ticket 14).

**Idea (the user, 2026-10-25):** a square version of the current drift view, 128 by 128. The zero line runs across the middle. As keys drift up or down, their line gets longer.

## Where things stand

- `drift-view.js` draws a header line, a lane of bars (up for sharp, down for flat, from a centre line) and a keyboard of 128 keys, 960 by 200. A Picture mode shows a 16 by 8 landscape instead.
- Bar length and colour use a log scale up to an octave (`FULL_SCALE_CENTS = 1200`), so a few cents still show.
- `tap.js` sends 128 offsets in cents whenever the pitch table changes. The new view needs no engine change.
- The reading below is what's assumed until confirmed: 128 columns, one per key from note 0 at the left to 127 at the right. Each column is a vertical line from the centre that grows up when the key is sharp and down when it is flat.

## Decided with the user (2026-10-26, round 1)

- **The line runs from bottom left to top right,** not across the middle. The user changed their mind about the centre line. Read as a pitch map: across is the key, up is the pitch it plays, so concert pitch is the diagonal. To confirm in round 2.
- **Pixel size and full scale:** open again, because the diagonal changes them.
- **Colour:** red when sharp, blue when flat, deeper the further.
- **No hover readout.**
- **Replaces Keys and Picture for now.** The user wants ideas for other ways to show or warp the drift.
- **Jump** to the new state on each change. No animation.

## Decided with the user (2026-10-26, round 2)

- **A pitch map, confirmed.** Across is the key (note 0 at the left), up is the pitch it plays (note 0 at the bottom). Concert pitch is the diagonal.
- **Drawn as a faint diagonal guide plus a stem** from the diagonal to each drifted key's pitch, red when sharp and blue when flat. Keys at concert pitch draw no stem.
- **Other views** go to a parked ticket, [18](18-other-views.md). The user liked the pitch-class circle, the rubber sheet and the warped picture.

## Questions to grill

1. **Is that reading right?** One column per key, low notes at the left, with lines growing up and down from a horizontal middle line? *Recommendation:* confirm with a quick mock-up.
2. **Pixel size:** literally 128 by 128 pixels (one pixel per key), or square and scaled up (for example 256 by 256, two pixels per key)? *Recommendation:* scaled up, at a whole number of pixels per key, so lines stay crisp. 128 pixels is small on a laptop screen.
3. **Full scale:** how much drift reaches the top or bottom edge? ±12 semitones, ±2 (the range the user liked for Move), or the Lowest and Highest bounds? *Recommendation:* ±12 semitones. Ask whether the scale should be linear (simple to read) or log as now (a few cents still show).
4. **Past full scale:** clip at the edge, or mark the line (for example a brighter tip)? *Recommendation:* clip, with a brighter tip.
5. **Colour:** red up and blue down as now, one colour, or colour by how far? *Recommendation:* keep red and blue, deeper the further.
6. **Extras:** show keys that are held, the last key released, or the Lowest and Highest bounds as faint vertical lines? *Recommendation:* the bounds and the last key released. Held keys later.
7. **Hover:** keep the readout ("C3 +300.0 cents (plays near D#3)")? *Recommendation:* yes, in the panel's header line.
8. **Keys and Picture views:** keep them as switchable views, or does the square replace them? *Recommendation:* the square becomes the default. Keep Picture as a switch, since the user liked it. Drop Keys if the square covers it.
9. **Smoothing:** jump to the new length on each change, or animate briefly? *Recommendation:* jump. It is cheaper and shows exactly when a release lands.

## Done when grilled

- Answers recorded here.
- Rewritten as a build ticket. It changes `drift-view.js` (or adds a new `jsui` file) and its tests.
