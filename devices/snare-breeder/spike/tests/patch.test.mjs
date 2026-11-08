// Checks on the generated Snare Breeder spike device: the checks every breeder device shares
// (shared/breeder/tests/patch-checks.mjs), then the snare's own voice wiring.

import test from "node:test";
import assert from "node:assert/strict";
import * as build from "../build-patch.mjs";
import { checkBreederPatch, only } from "../../../../shared/breeder/tests/patch-checks.mjs";

const { KB, patch } = checkBreederPatch(build, new URL("../", import.meta.url));
const { boxes, targets } = patch;

test("the snare's settings reach the voice in unpack order", () => {
  const unpack = only(boxes.filter((b) => b.text === "unpack 0. 0. 0. 0. 0. 0. 0. 0."), "settings unpack");
  const into = (outlet) => only(targets(unpack.id, outlet), `unpack outlet ${outlet}`);
  assert.deepEqual([0, 1, 2, 3, 4, 5, 6, 7].map((o) => into(o).text),
    ["*~ 1.6", "*~ 0.5", "svf~ 3000 0.4", "svf~ 3000 0.4", "*~ 1.", "*~ 1.", "lores~ 8000 0.", "*~ 0.5"]);
  const line = (outlet, inlet) => patch.lines.some((l) => l.source[0] === unpack.id && l.source[1] === outlet && l.destination[1] === inlet);
  assert.ok(line(2, 1) && line(3, 2), "wires tone to the cutoff inlet, resonance to the resonance inlet");
  assert.equal(KB.voice(KB.PARAMS.map(() => 0.5), false).statics.length, 8);
});

test("the snap filter in the patch is the one the loudness simulation uses", () => {
  const snapFilter = only(boxes.filter((b) => b.text === `svf~ ${KB.SNAP_FREQ} ${KB.SNAP_RES}`), "snap svf~");
  assert.ok(patch.lines.some((l) => l.source[0] === snapFilter.id && l.source[1] === 1), "high-pass outlet");
});

test("both oscillators start at phase 0.75 on every hit", () => {
  const phase = only(boxes.filter((b) => b.maxclass === "message" && b.text === "0.75"), "phase message");
  const oscs = targets(phase.id, 0);
  assert.equal(oscs.length, 2);
  assert.ok(oscs.every((b) => b.text === "cycle~"));
});
