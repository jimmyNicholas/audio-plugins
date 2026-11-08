# Kick Breeder spike

A throwaway Max for Live **instrument** for Live 11 (Max 8). It answers one question:

> If you choose between kicks round after round, with new ones bred from each choice, do you land on a kick you like within about ten rounds?

Every sound is synthesised: a swept sine, a noise click, drive and a low-pass. No samples. Spec: [`../docs/SPEC.md`](../docs/SPEC.md).

## Files

| File | What it is |
| --- | --- |
| `Kick Breeder Spike.amxd` | The device. Generated; load this one. |
| `Kick Breeder Spike.maxpat` | The same patch as readable JSON. Generated. |
| `kick-breeder.js` | Breeding engine, kick envelope maths and Max glue (ES5). Must sit next to the `.amxd`. |
| `build-patch.mjs` | Writes the two device files: `npm run build`. |
| `tests/` | Node tests for the engine and the patch: `npm test`. |

## How the device works

- **Breed mode:** round 1 has slots A and B. Press **Hear** to play a slot; **Choose** keeps that kick as the favourite and breeds two variations, so later rounds have A, B and C. The favourite moves between slots unless Shuffle is off.
- **MIDI notes** play the slot you last heard, so a running clip lets you compare kicks in a beat.
- **Keep** copies the slot you last heard into the nine **Kept** dials and switches **Mode** to Kept. From then on MIDI plays the kept kick, the dials tweak it, and it saves with the set.
- **Breed From Kept** starts a new search around the kept kick.
- **Locks** under each dial stop that setting changing in new variations.
- **Wider / Closer** make the variations more or less different. **Back** undoes a round. **Fresh** starts again.
- **Match level** plays every kick at about the same loudness (worked out by a rough simulation in the js). **Show fav** marks the favourite's slot.

## Set up

1. In Live's browser, **Places > Add Folder**, and choose this `spike` folder in your checkout.
2. Make a **MIDI track** and drag **Kick Breeder Spike.amxd** onto it. It is an instrument, so it goes where Simpler or Operator would.
3. Make a one-bar MIDI clip with a note on every beat (any pitch). Loop it.
4. For errors: click the device's **Edit** button (the small icon in its title bar) and open **Window > Max Console** in Max. Copy any red lines into your notes.

## Checks

Fill in the Pass column (y / n / a note) and send it back.

| # | Do | Expect | Pass |
| --- | --- | --- | --- |
| 1 | Load the device | The status line reads `Round 1 \| choices 0 \| variation 0.25 \| hearing A`. Slot C reads `(none)`. No red lines in the Max Console | [ ] |
| 2 | Press **Hear A**, then **Hear B** | Two different kicks. The line under the status describes each one | [ ] |
| 3 | Play the clip, then press **Hear A** and **Hear B** while it runs | The clip plays whichever slot you heard last | [ ] |
| 4 | **Choose** either slot | Round 2, and slot C now has a kick | [ ] |
| 5 | Turn on **Show fav**. **Choose** the slot marked favourite | Variation drops from 0.25 to 0.15 | [ ] |
| 6 | Turn on **Lock** under **Decay**. **Choose** a few times, watching the decay in the description | Decay stays the same | [ ] |
| 7 | Turn **Match level** off for a few rounds, then on | Off: some kicks are much louder. On: they sit at about the same level. Note whether it affected which you picked | [ ] |
| 8 | Try **Wider**, **Closer**, **Back** and **Fresh** | Wider: slots more different. Closer: more alike. Back: the previous round. Fresh: round 1 again | [ ] |
| 9 | When you like a kick, **Hear** it, then **Keep** | Mode switches to **Kept**, the dials move, the clip plays that kick, and the status says how many choices it took | [ ] |
| 10 | Turn a **Kept** dial while the clip plays | The kick changes | [ ] |
| 11 | Save the set, close it, open it again | Mode is still Kept and the same kick plays | [ ] |
| 12 | **Breed From Kept** | Back in Breed mode, with the kept kick as the favourite and two close variations | [ ] |
| 13 | Listen to a fast clip (16th notes) for a while | Note any clicks between hits, or hits that feel late | [ ] |

## The spike's answer

Do a few full searches from **Fresh** to **Keep** and note:

| Run | Choices to Keep | How it felt (too slow, too random, stuck, good) |
| --- | --- | --- |
| 1 | | |
| 2 | | |
| 3 | | |

Also worth a line each:

- Did you trust your choice more with **Match level** on?
- Did knowing the favourite (**Show fav**) pull you towards it?
- Is anything missing from the kick itself: a sub, a second body layer, longer clicks?

## Known limits

- The breeding history is not saved with the set. Only the Kept kick is.
- Loudness matching is based on a simulation of the patch, so levels are close, not exact.
- Turning a Kept dial reruns that simulation in the js, so dragging a dial fast may lag a little.
- Note off is ignored: each kick rings out. A new note restarts the kick, which can click if the last one is still sounding.
