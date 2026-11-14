# Releases

How work in this repo becomes something released: albums of plugins, with the music, writing and manifests around them. This glossary covers the words the business plan, the album maps and their tickets share.

## Release

**Album**:
A themed release: 3 to 6 plugins, music made with them, a sample pack made from them, the story behind them and art. Numbered in release order; album 1 is Bloodline Breaks.
_Avoid_: Bundle, pack, collection (a collection is one stage of an album)

**Collection**:
An album's first stage: rough Max for Live spikes, one per sound, gathered while the album's ideals are written down. Broader and rougher than the plugin set it is refined into.
_Avoid_: Plugin set, range

**Plugin set**:
The plugins an album releases: refined from its collection (polished, merged or cut) and built in JUCE, each with a free browser version.
_Avoid_: Collection, product line

**Spike**:
A quick, throwaway build that answers a question by being played. Max for Live spikes make up a collection; a JUCE spike tests how a plugin will be built.
_Avoid_: Prototype (for builds), demo, MVP

**Ideals**:
What an album believes about making music, written down during its collection stage and grown into its essay, liner notes and each plugin's "why" note.
_Avoid_: Manifesto, mission, philosophy doc

**Manifest**:
The YAML file that lists what a release contains and gates it: `album.yaml` for an album, `release.yaml` for a plugin.
_Avoid_: Manifesto, catalogue, metadata

## Breeding

**Breeder**:
A sound source you set by choosing between candidates, round after round, rather than by turning knobs. Kick Breeder and Snare Breeder are breeders.
_Avoid_: Generator, randomiser, evolver

**Drum**:
One kind of sound a breeder makes, such as kick, snare or hats. In a plugin with one shared voice, a drum is a zone within that voice's settings: choosing one centres breeding there, but any sound stays reachable. With no drum chosen, breeding starts anywhere. One plugin can hold several drums.
_Avoid_: Voice (the synthesis code for a drum), instrument, pad

**Row**:
One breeding lane in a breeder plugin's pad grid: three sibling candidates (A, B, C) and a fourth pad, with its own zone, spread, favourite and locks. A row is either open or placed, and sits on the workbench or in a kit slot.
_Avoid_: Lane, slot (a slot is where a row sits), track

**Open**:
A row that is breeding: its fourth pad is Choose, and its zone and locks can be changed. The workbench row is always open; a placed row can be opened where it sits and closed again.
_Avoid_: Editing, unlocked

**Placed**:
A row that is played, not bred: its fourth pad is Random, which plays one of its three siblings, never the same twice in a row.
_Avoid_: Saved, finished, frozen

**Workbench**:
The bottom row of the grid, where new rows are bred. It is always open, and starts again with no zone once its row is placed in the kit.
_Avoid_: Editor, bench row

**Kit**:
The slots above the workbench, each blank or holding a placed row. A row's sound follows its slot, so moving a row changes what a clip on that slot plays.
_Avoid_: Rack (an Ableton word), crate, pad, bank

**Start again**:
Wiping a whole breeder plugin back to an empty kit and a fresh workbench. Resetting one open row instead draws three fresh candidates and keeps its zone and locks.
_Avoid_: Kit reset, clear (emptying one slot)

**Favourite**:
A row's last chosen candidate. It stays on its pad while the other two pads get new sounds bred from it.
_Avoid_: Winner, keeper, parent

**Spread**:
How far new candidates are bred from the favourite. Choosing the favourite again narrows it; choosing a new candidate twice in a row widens it.
_Avoid_: Step size (the code's name for it), mutation rate, Wider and Closer
