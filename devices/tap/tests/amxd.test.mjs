import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { pack, unpack, readChunks } from "../tools/amxd.mjs";

const patcher = JSON.stringify({ patcher: { fileversion: 1, boxes: [], lines: [] } }, null, 2) + "\n";

test("pack writes ampf, meta and ptch chunks for a MIDI effect", () => {
  const chunks = readChunks(pack(patcher));
  assert.deepEqual(
    chunks.map((c) => c.tag),
    ["ampf", "meta", "ptch"],
  );
  assert.equal(chunks[0].data.toString("ascii"), "mmmm");
  assert.equal(chunks[1].data.length, 4);
  assert.equal(chunks[2].data[chunks[2].data.length - 1], 0);
});

test("unpack returns the patcher text", () => {
  assert.equal(unpack(pack(patcher)), patcher);
});

test("pack rejects broken JSON", () => {
  assert.throws(() => pack("{ nope"));
});

test("unpack refuses a frozen device", () => {
  const ptchHeader = Buffer.from("ptch\0\0\0\0", "ascii");
  ptchHeader.writeUInt32LE(4, 4);
  const frozen = Buffer.concat([pack(patcher).subarray(0, 24), ptchHeader, Buffer.from("mx@c")]);
  assert.throws(() => unpack(frozen), /frozen/);
});

for (const name of ["spike/TAP MPE Spike"]) {
  test(`${name}.amxd is in sync with its .maxpat`, () => {
    const source = fs.readFileSync(new URL(`../${name}.maxpat`, import.meta.url), "utf8");
    const device = fs.readFileSync(new URL(`../${name}.amxd`, import.meta.url));
    assert.ok(Buffer.compare(pack(source), device) === 0, "run: npm run build --prefix m4l");
  });
}
