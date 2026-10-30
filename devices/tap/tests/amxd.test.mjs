import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { pack } from "../../../tools/amxd.mjs";

for (const name of ["TAP", "spike/TAP MPE Spike"]) {
  test(`${name}.amxd is in sync with its .maxpat`, () => {
    const source = fs.readFileSync(new URL(`../${name}.maxpat`, import.meta.url), "utf8");
    const device = fs.readFileSync(new URL(`../${name}.amxd`, import.meta.url));
    assert.ok(Buffer.compare(pack(source), device) === 0, "run: npm run build --prefix devices/tap");
  });
}
