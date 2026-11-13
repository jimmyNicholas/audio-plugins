// Checks on the generated Kick Breeder spike device: the checks every breeder device shares
// (shared/breeder/tests/patch-checks.mjs), then the kick's own voice wiring.

import test from "node:test";
import assert from "node:assert/strict";
import * as build from "../build-patch.mjs";
import { checkBreederPatch, only } from "../../../../shared/breeder/tests/patch-checks.mjs";

const { patch } = checkBreederPatch(build, new URL("../", import.meta.url));
const { boxes, targets } = patch;

test("the kick's settings reach the voice in unpack order", () => {
  const unpack = only(boxes.filter((b) => b.text === "unpack 0. 0. 0. 0. 0. 0."), "settings unpack");
  const into = (outlet) => only(targets(unpack.id, outlet), `unpack outlet ${outlet}`).text;
  assert.deepEqual([0, 1, 2, 3, 4, 5].map(into), ["svf~ 3000 0.3", "svf~ 2000 0.", "*~ 1.", "*~ 1.", "lores~ 8000 0.", "*~ 0.5"]);
});
