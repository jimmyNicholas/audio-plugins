# Spec: Bloodline Track, an HTML prototype

Status: ready for agent. Written 2026-11-16 from the conversation that produced the [clave, subdivision and groove data note](../research/clave-subdivision-and-groove-data.md).

Builds on the last HTML version, the Bloodline Breaks mobile prototype (Drums, Groove and Break tabs under a sticky transport). This is a throwaway prototype in the sense of the prototype skill: it answers design questions about flow and layout, and is not production code.

## Problem Statement

In the Breaks mobile prototype, a user can breed drums, breed a groove and breed a break, but the result is a single drum loop with nowhere to go. They cannot add a bass line or a synth part to the same loop, mix the parts against each other, or put effects on them. Global choices that affect everything (the groove, the overall level) sit inside the drum workflow, as if they belonged only to the break.

Two things in the break itself also get in the way:

- **The "Notes per bar" card** (Quarters, Eighths, Sixteenths, Total per row) is fiddly and has not earned its place. The groove research suggests busyness comes from bars whose hits do not belong together, not from the number of hits, so counting per subdivision is the wrong control.
- **A drum's role is fixed by the zone it was bred in.** A drum bred under "None" or "Perc" that actually sounds like a snare cannot be told to act as one, so the break writes the wrong part for it (the break decides what a row plays, kick, snare, hat or perc, from the pad's zone).

## Solution

A new HTML prototype, Bloodline Track, made from the Breaks mobile prototype.

- A **Track** section becomes the home screen. It shows the loop's parts (drums, bass, synth) as a list of lanes with a mixer, effects, and the global settings: groove, tempo, bars, and levelling.
- **Adding a part** walks the user through two steps: first breed the sound (drums for a drum part, a bass or synth voice for a pitched part), then breed its pattern or loop. Finishing drops the part into the Track as a new lane.
- **The break loses its subdivision counts.** The Notes per bar card goes.
- **Kept drums can be relabelled.** A kept pad's label (Kick, Snare, Hats, Perc, None) can be changed after it is kept, and the break uses the new label to decide what that row plays.

It is built in two phases, as the Breaks mobile prototype was: first the UI, shown as diffs against the last version with no sound, then the engine hooked up.

## User Stories

### Track section

1. As a producer, I want the Track to be the first thing I see, so that I start from the whole loop rather than one drum.
2. As a producer, I want to see every part in the loop as a lane (drums, bass, synth), so that I know what is playing.
3. As a producer, I want each lane to show its name, kind and a small picture of its pattern, so that I can tell lanes apart at a glance.
4. As a producer, I want an "Add part" action offering Drums, Bass and Synth, so that I can build the loop up one part at a time.
5. As a producer, I want an empty Track to show a clear prompt to add the first part, so that a new user knows where to begin.
6. As a producer, I want to open a lane to change its sound or its pattern again, so that a part is never finished for good.
7. As a producer, I want to remove a lane, so that I can drop parts that do not work.
8. As a producer, I want to duplicate a lane, so that I can try a variation without losing the original.
9. As a producer, I want to rename a lane, so that the mix reads the way I think about it.
10. As a producer, I want to reorder lanes, so that related parts sit together.
11. As a mobile user, I want the Track to work at phone width with no sideways scrolling, so that I can use it on my phone.
12. As a desktop user, I want the Track and the selected lane's detail side by side on a wide screen, so that I can mix while editing.

### Mixing

13. As a producer, I want a level fader on each lane, so that I can balance the parts.
14. As a producer, I want a pan control on each lane, so that I can place parts in the stereo field.
15. As a producer, I want mute and solo on each lane, so that I can hear parts on their own or without them.
16. As a producer, I want a meter on each lane and on the master, so that I can see what is loud.
17. As a producer, I want a master level and the -1 dB safety limiter on the master, so that the loop never clips.
18. As a producer, I want to see when the limiter is working and by how much, so that I know when to turn things down.
19. As a producer, I want an "Auto level" action that sets lane levels so nothing dominates and the limiter stays idle, so that I get a usable balance in one tap.
20. As a producer, I want Auto level to respect lanes I have locked, so that a level I chose by ear is kept.
21. As a producer, I want drum rows inside a drum lane to keep their own levels, so that the kit balance within the break is not lost when I mix the lane.

### Effects

22. As a producer, I want an effects slot on each lane, so that I can shape a part's sound in context.
23. As a producer, I want a master effects slot, so that I can glue the whole loop together.
24. As a producer, I want to breed effects chains from a character (for example warm, dusty, wide), using the same litter of four with Breed, Cross, Back and New, so that effects work the way sounds do.
25. As a producer, I want to bypass an effects chain, so that I can compare with and without it.
26. As a producer, I want to lock an effect's settings, so that breeding keeps the parts I like.
27. As a producer, I want tempo-synced delay and gate times to follow the Track tempo, so that effects stay in time when I change BPM.

### Global groove and timing

28. As a producer, I want the groove to live in the Track, so that one feel applies to every part.
29. As a producer, I want to breed grooves (Groove, Complexity, Forward, Behind) in the Track exactly as I did in the Groove tab, so that the feature I know is still there.
30. As a producer, I want the chosen groove to set swing and push or pull for every lane, drums and pitched parts alike, so that bass and drums sit together.
31. As a producer, I want to set a lane's feel (Forward, On, Behind) per lane in the Track, and per row inside a drum lane, so that I can lay the bass behind the kick or push the hats.
32. As a producer, I want to opt a lane out of the groove's swing, so that a part can stay straight on purpose.
33. As a producer, I want tempo and bars (2 or 4) set once in the Track, so that every part loops together.
34. As a producer, I want "Less swing at faster tempos" as a Track setting, so that the feel stays natural when I change tempo.
35. As a producer, I want to keep grooves in slots as before, so that I can return to a feel I liked.

### Adding a part: sound, then pattern

36. As a producer, I want adding a part to start with breeding its sound, so that I choose how it sounds before what it plays.
37. As a producer, I want the steps shown (Sound, then Pattern) with where I am, so that I know how far through I am.
38. As a producer, I want to go back from Pattern to Sound without losing my pattern litter, so that I can change the sound and hear the same pattern.
39. As a producer, I want to cancel adding a part, so that a half-made part does not end up in the loop.
40. As a producer, I want to hear the rest of the Track playing while I breed a new part, so that I choose it in context.
41. As a producer, I want the option to hear the new part alone while breeding, so that I can judge it on its own.
42. As a producer, I want "Add to track" at the end of the Pattern step, so that the part lands in the Track as a new lane.

### Drum parts

43. As a producer, I want the drum Sound step to be the existing Drums pane (zones, litter, settings, locks, pads 36 to 43), so that drum breeding is unchanged.
44. As a producer, I want the drum Pattern step to be the existing Break pane (rows, bars view, step editing, row locks, kept loops), without the Notes per bar card, so that the break is simpler.
45. As a producer, I want to change a kept pad's label after keeping it, so that a drum bred as "None" or "Perc" can act as a kick, snare or hat in the break.
46. As a producer, I want to relabel a pad from the pad itself and from the row it plays in the break, so that I can fix a wrong part where I notice it.
47. As a producer, I want a relabelled pad's colour and name tag to change straight away, so that the break shows what each row now is.
48. As a producer, I want the break to write the relabelled row's part on the next Breed or New, so that my existing loop is not overwritten without asking.
49. As a producer, I want a "Rewrite row" action after a relabel, so that I can apply the new role to the current loop at once if I want to.
50. As a producer, I want relabelling to leave the drum's sound alone, so that only what it plays changes.
51. As a producer, I want two rows with the same label to be allowed (for example two hats), so that I am not forced into one of each.
52. As a producer, I want more than one drum lane in a Track, so that I can layer a second kit or a percussion part.

### Bass and synth parts

53. As a producer, I want the Bass and Synth Sound steps to breed a voice from zones (for example sub, pluck, pad, lead) with the litter of four, so that pitched sounds work like drums.
54. As a producer, I want to audition a pitched sound on a short phrase in the Track's key, so that I hear it playing notes, not a single hit.
55. As a producer, I want the Pattern step to breed a pitched loop from traits (as in Bloodline Synth 2), so that I get lines that fit without writing notes.
56. As a producer, I want key and scale set once in the Track, so that bass and synth parts agree.
57. As a producer, I want a bass loop to be able to follow the kick of a drum lane I choose, so that bass and kick lock together.
58. As a producer, I want to see a pitched loop as a small note grid, so that I can see its shape.
59. As a producer, I want to keep pitched sounds and loops in slots, as with drums, so that I can return to them.

### Playback and export

60. As a producer, I want one Play for the whole Track, so that I hear the loop as it is mixed.
61. As a producer, I want changes to keep playing in place (same position in the loop), so that tweaking does not restart the loop.
62. As a producer, I want to export the Track mix as a WAV, so that I can use it elsewhere.
63. As a producer, I want to export a zip with the mix, a stem per lane and the Track as JSON, so that I can remix it in a DAW.
64. As a producer, I want stems to add up to the mix, so that nothing changes when I rebuild it.
65. As a producer, I want exported loops to wrap tails round to the start, so that they loop seamlessly.

### History

66. As a producer, I want Back to work per litter as it does now, so that breeding mistakes are easy to undo.
67. As a producer, I want Undo for Track edits (level, pan, remove, relabel), so that a mixing mistake is easy to reverse.

## Implementation Decisions

### Build order

- **Phase 1, UI diffs.** Make the new screens as a silent mock first, as with the first Breaks mobile commit. Each changed screen is presented as a diff against the last version: what is removed (Notes per bar card, Groove tab), what moves (Groove into Track), and what is new (Track, Add part flow, relabel, mixer, effects, pitched parts). A screen of before and after pairs, or a toggle between them, is enough. The user reviews this before any engine work.
- **Phase 2, engine.** Hook the mock up to sound, reusing code from the Breaks mobile prototype (drum voice, break writer, groove, offline render, limiter, export) and the shared Bloodline engine (synth voice, synth loops from traits, effects chains, litter).

### Structure

- **Sections:** Track (home), and a per-part editor with two steps, Sound and Pattern. The Groove tab is removed as a tab; its contents move into the Track's global area. On phones the tab bar becomes Track plus the open part's steps; from about 1000 px wide, Track and the open part sit side by side.
- **Lane kinds:** drums, bass, synth. A drum lane holds up to four rows, played by kept pads, as the break does now. Bass and synth lanes hold one voice and one note loop.
- **Global settings (Track):** tempo, bars (2 or 4), key and scale, groove (bred and kept), Less swing at faster tempos, master effects, master level, limiter.
- **Per lane:** name, kind, level, pan, mute, solo, feel (Forward, On, Behind), follow groove swing (on by default), effects chain, level lock for Auto level.
- **Shape of the project state** (a sketch, not a contract; trim or rename freely):

```js
track = {
  bpm, bars, key, scale, easeSwing,
  groove: { litter, kept, seedFrom },          // as in the Groove pane today
  master: { level, fx },
  lanes: [
    { id, name, kind: 'drums' | 'bass' | 'synth',
      level, pan, mute, solo, feel, followSwing, levelLocked, fx,
      sound:   /* drums: litter + pads[8] with { name, v, label }; pitched: litter + kept */,
      pattern: /* drums: loop litter + kept loops + row locks; pitched: note-loop litter + kept */ },
  ],
  adding: null | { kind, step: 'sound' | 'pattern', draft },   // the Add part flow
}
```

### Relabelling

- A kept pad stores its **label** separately from the **zone** it was bred in. The zone still bounds breeding of that drum; the label decides the row's part in the break (the current zone to role map, Kick to kick, Snare to snare, Hats to hat, Perc to perc, None to drum, is applied to the label instead).
- Relabelling changes colour and tag at once and takes effect on the next Breed or New of the loop. A "Rewrite row" action writes the row now from the new role, respecting row locks.
- Mutating the drum's sound afterwards keeps the label.

### Removing subdivisions

- The Notes per bar card, its state and its Fit action are removed, along with the shaped candidate and shaped mutate paths that served them. Breeding falls back to the unshaped path.
- The break's density is still steered by Complexity from the groove. Improving how the break writes bars (anchors, a core bar, a change budget, per the research note) is a separate piece of work, see Out of Scope.

### Groove across parts

- The seed groove's swing, accents and drift apply to every lane at render time. Swing applies to pitched notes on the same subdivision grid as drums. Feel offsets (Forward, Behind) apply per lane, and per row within a drum lane, using the same millisecond and brightness rules the Groove pane describes today.
- Kick rows staying on time (the current rule) is kept.

### Mixing and levelling

- Each lane renders to its own stereo stem; lane level, pan, mute and solo are applied after the stem is rendered; the master sums stems, runs master effects, then the -1 dB limiter (the current two-pass, loop-settling limiter).
- **Auto level:** sets unlocked lane levels from each stem's loudness so lanes sit near a common target, then trims all lanes together so the master peak sits just under the limiter. Within a drum lane, row levels are left alone.
- Export follows the current rules: mix as WAV; zip of mix, one stem per lane, and the Track as JSON; stems sum to the mix.

### Effects

- Effects chains come from the shared engine's effects (types, characters, fresh, mutate, cross), bred with the same litter of four. Delay and gate times are in beats and follow tempo.

### Add part flow

- A step header shows Sound then Pattern. Back from Pattern keeps the pattern litter. Cancel discards the draft. Add to track appends the lane and returns to the Track with it selected.
- While adding, playback plays the Track with the draft lane mixed in; a Solo toggle plays the draft alone. Pattern breeding for a bass lane can take a chosen drum lane's kick as a guide.

## Testing Decisions

### Seams

- **One seam: the project state and its offline render.** All behaviour is reached through two things: actions applied to the Track state (add part, step, relabel, breed, mix changes, undo), and the offline render of a Track to stems and a mix. This mirrors the Breaks mobile prototype, where the offline render already exists and the UI only draws state.
- No tests below that seam (voices, single effects, the litter internals) beyond what already exists in the shared engine.

### What a good test is here

- Tests check behaviour a user could notice: what a row plays after a relabel, that stems sum to the mix, that the limiter keeps the master peak at or under -1 dB, that a muted lane is silent, that Auto level leaves locked lanes alone, that removing the subdivision card leaves breeding working. They do not check internal function names or the order of internal calls.
- Randomness is seeded, so a test can fix a seed and check outcomes.

### Which parts are tested

- Phase 1 (UI diffs): no automated tests. A Playwright screenshot at phone width and at desktop width of each screen, for review.
- Phase 2 (engine): through the one seam, driven in a headless browser (Playwright `page.evaluate` against the page's exposed state and actions) or in Node if the state and render code is loaded from a separate script.

### Prior art

- The Breaks mobile prototype's offline render and export (stems summing to the mix, limiter headroom) are the behaviour to match.
- The repo's plugin tests (for example the Bloodline Breaks and Breaks 2 test suites) test pattern behaviour from the outside in the same spirit: what a pattern contains, not how it was built.
- `tools/amxd.test.mjs` is prior art for small Node tests in this repo.

## Out of Scope

- Rewriting how the break writes bars (anchor or clave layer, a repeated core bar per row, a per-bar change budget, smaller default swing, per-row timing offsets). The research note proposes this; it should be its own spec once this prototype settles the flow.
- Porting any of this to the JUCE plugins.
- Arrangement beyond one loop (song sections, scenes, chains of loops).
- Automation of mix or effect settings over time.
- MIDI export of patterns.
- Recording, audio import or samples.
- Saving projects beyond export (no accounts, no cloud storage). Browser storage only for conveniences, if at all.

## Further Notes

- The last HTML version also carries two leaks found by the research (the kick on the one sometimes missing, and "0" steps still firing in the plugin's style tables). The first lives in the plugin code rather than this prototype; the prototype's own writer keeps beat 1 and the backbeat. Worth checking while porting, but not required here.
- The research found that funk, hip hop and soul hats in real playing are close to straight (about 50 to 51%) and that kick and backbeat land a few milliseconds early against the hats. If the global groove's defaults are touched while moving it into the Track, these are the numbers to lean towards.
- Open question for the user: whether the Groove's Complexity should stay a global setting or move to each drum lane, since with several drum lanes one Complexity may not fit all.
