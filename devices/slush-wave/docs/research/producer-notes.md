# Producer notes on slushwave

Shared by the user on 2026-10-28. Two sources: a producer's tips (unnamed), and a summary of a tutorial by Desert Sand Feels Warm at Night. Neither gives exact plugin settings beyond what is quoted here. The user prefers a sample based process, and likes the tutorial's idea of bouncing with effects and then pitching down.

## Producer tips

1. **Barberpole phasing, subtle.** Little to no feedback. "Fingers tracing water, not an F-15 flying past your head."
2. **Split the source into at least three channels:**
   - **Left:** panned 75% left, stereo width about 1/3, phasing upward.
   - **Right:** panned 75% right, stereo width about 1/3, phasing downward.
   - **Centre:** highs cut by EQ, compressed to tighten, width 1/3 or less.
3. **Centre (bass) channel:** no reverb, a little distortion. The left and right channels play off each other.
4. **Paulstretch layer.** Paulstretch some source material, pick the best moments, and bring them back in as an ambient overlay, with reverb and EQ so it sits with the original.
5. **Reverb and filtered delays** make everything better when applied well, but work with the sound raw (even at 1x) and A/B against the effects to check direction.
6. **Be more avant garde with sample editing** than early t e l e p a t h. Creative editing and instrumentation make tracks stand out.

## Desert Sand Feels Warm at Night tutorial (summary)

Not sample based: he writes and records an original track at about 70 BPM, then slushes it. No DAW, plugin names or numeric settings are given unless stated.

- **Slowdown:** bounce, then slow from about 70 to about 57 BPM (about 0.81x, so pitch falls with tempo, about 3.6 semitones), and work in a project at 114 BPM (the slowed tempo on a doubled grid). He prefers lively, not too slow slush that keeps some harmonics. Check the slowed result early and often.
- **Effect chain, in order:**
  1. Delay (most audible on the snare).
  2. Phaser (first "layer", so everything after is coloured by it).
  3. Reverb 1: bright, tinny reverb harmonics with very little low end.
  4. EQ: boost bass, roll off from about 15 kHz up. After reverb 1 so its tail adds no boom.
  5. Main reverb: a lush "signature" reverb.
  6. Compressor, last.
- **Editing after slushing:** keep the project at the slowed tempo, loop sections with short fades at the joins, add a slow lead-in, often work in three bar phrases, then fade out.
- **Master:** a limiter only.
- **Timing:** loose timing on chords and melody adds a human feel that slowing exposes; bass and kick should be tight.

## What this changes for the map

- **Slowdown** ([#5](https://github.com/jimmyNicholas/audio-plugins/issues/5)): both sources slow a rendered or sampled source, not a live stream. Varispeed at about 0.8x is the reference. A sample based device sidesteps the live slowdown limit.
- **Stages and order** ([#6](https://github.com/jimmyNicholas/audio-plugins/issues/6)): adds a three way split (left phased up, right phased down, centre bass dry with distortion), a barberpole phaser with low feedback, and a second chain order to compare: delay, phaser, bright reverb, EQ, main reverb, compressor.
- **New idea:** a Paulstretch style ambient layer alongside the main sound.
