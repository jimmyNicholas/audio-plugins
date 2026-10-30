import test from "node:test";
import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { pack, unpack, readChunks, MIDI_EFFECT, AUDIO_EFFECT } from "./amxd.mjs";

const tool = fileURLToPath(new URL("./amxd.mjs", import.meta.url));

function patcherOf(project) {
  return JSON.stringify({ patcher: { fileversion: 1, project, boxes: [], lines: [] } }, null, 2) + "\n";
}

const midiPatcher = patcherOf({ amxdtype: 1835887981 });
const audioPatcher = patcherOf({ amxdtype: 1633771873 });

test("pack writes ampf, meta and ptch chunks for a MIDI effect", () => {
  const chunks = readChunks(pack(midiPatcher));
  assert.deepEqual(
    chunks.map((c) => c.tag),
    ["ampf", "meta", "ptch"],
  );
  assert.equal(chunks[0].data.toString("ascii"), "mmmm");
  assert.equal(chunks[1].data.length, 4);
  assert.equal(chunks[2].data[chunks[2].data.length - 1], 0);
});

test("pack writes the aaaa tag for an audio effect", () => {
  const chunks = readChunks(pack(audioPatcher, AUDIO_EFFECT));
  assert.equal(chunks[0].data.toString("ascii"), "aaaa");
});

for (const [type, patcher, other] of [
  [MIDI_EFFECT, midiPatcher, audioPatcher],
  [AUDIO_EFFECT, audioPatcher, midiPatcher],
]) {
  test(`pack as ${type} rejects a patcher saved as another device type`, () => {
    assert.throws(() => pack(other, type), /amxdtype/);
  });

  test(`pack as ${type} rejects a patcher with no amxdtype`, () => {
    assert.throws(() => pack(patcherOf({}), type), /amxdtype/);
    assert.throws(() => pack(JSON.stringify({ patcher: { boxes: [] } }), type), /amxdtype/);
  });

  test(`pack as ${type} accepts a patcher saved as that type`, () => {
    assert.doesNotThrow(() => pack(patcher, type));
  });
}

test("unpack returns the patcher text", () => {
  assert.equal(unpack(pack(midiPatcher)), midiPatcher);
});

test("pack rejects broken JSON", () => {
  assert.throws(() => pack("{ nope"));
});

test("unpack refuses a frozen device", () => {
  const ptchHeader = Buffer.from("ptch\0\0\0\0", "ascii");
  ptchHeader.writeUInt32LE(4, 4);
  const frozen = Buffer.concat([pack(midiPatcher).subarray(0, 24), ptchHeader, Buffer.from("mx@c")]);
  assert.throws(() => unpack(frozen), /frozen/);
});

function cli(args, files) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "amxd-"));
  for (const [name, text] of Object.entries(files)) fs.writeFileSync(path.join(dir, name), text);
  try {
    execFileSync(process.execPath, [tool, ...args], { cwd: dir, stdio: "pipe" });
    return { ok: true, dir };
  } catch (error) {
    return { ok: false, dir, stderr: error.stderr.toString() };
  }
}

test("CLI packs a MIDI effect by default", () => {
  const { ok, dir } = cli(["pack", "in.maxpat", "out.amxd"], { "in.maxpat": midiPatcher });
  assert.ok(ok);
  assert.ok(Buffer.compare(fs.readFileSync(path.join(dir, "out.amxd")), pack(midiPatcher)) === 0);
});

test("CLI packs an audio effect with --type audio", () => {
  const { ok, dir } = cli(["pack", "--type", "audio", "in.maxpat", "out.amxd"], { "in.maxpat": audioPatcher });
  assert.ok(ok);
  assert.ok(Buffer.compare(fs.readFileSync(path.join(dir, "out.amxd")), pack(audioPatcher, AUDIO_EFFECT)) === 0);
});

test("CLI fails when --type does not match the patcher", () => {
  const { ok, dir, stderr } = cli(["pack", "--type", "audio", "in.maxpat", "out.amxd"], { "in.maxpat": midiPatcher });
  assert.ok(!ok);
  assert.match(stderr, /amxdtype/);
  assert.ok(!fs.existsSync(path.join(dir, "out.amxd")));
});

test("CLI rejects an unknown --type", () => {
  const { ok, stderr } = cli(["pack", "--type", "instrument", "in.maxpat", "out.amxd"], { "in.maxpat": midiPatcher });
  assert.ok(!ok);
  assert.match(stderr, /Usage/);
});

test("CLI unpacks", () => {
  const { ok, dir } = cli(["unpack", "in.amxd", "out.maxpat"], { "in.amxd": pack(midiPatcher) });
  assert.ok(ok);
  assert.equal(fs.readFileSync(path.join(dir, "out.maxpat"), "utf8"), midiPatcher);
});
