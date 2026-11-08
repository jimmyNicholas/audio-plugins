// Checks on the generated Kick Breeder spike device. Nobody can run Max here, so these read the
// patcher JSON: files in sync, the device type, known objects only, valid lines, and that the
// panel and the js agree on message names, scripting names and ranges.

import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { createRequire } from "node:module";
import { pack, INSTRUMENT } from "../../../../tools/amxd.mjs";
import { FILE, JS_FILE, KEPT, MESSAGES, patcherText } from "../build-patch.mjs";

const require = createRequire(import.meta.url);
const here = (name) => new URL(name, import.meta.url);
const text = patcherText();
const patcher = JSON.parse(text).patcher;
const boxes = patcher.boxes.map((b) => b.box);
const byId = Object.fromEntries(boxes.map((b) => [b.id, b]));
const jsSource = fs.readFileSync(here(`../${JS_FILE}`), "utf8");
const KB = require(`../${JS_FILE}`);

// Standard Max 8 and MSP objects only. Check the Max reference before adding to this list.
const MAX_OBJECTS = new Set([
  "midiin", "midiparse", "unpack", "sel", "t", "/", "js", "prepend", "live.thisdevice", "deferlow",
  "line~", "cycle~", "*~", "+~", "noise~", "svf~", "tanh~", "lores~", "clip~", "plugout~",
]);

test("the committed .maxpat and .amxd match the builder", () => {
  assert.equal(fs.readFileSync(here(`../${FILE}.maxpat`), "utf8"), text, "run npm run build");
  assert.ok(Buffer.compare(fs.readFileSync(here(`../${FILE}.amxd`)), pack(text, INSTRUMENT)) === 0, "run npm run build");
});

test("it is an instrument with its panel open in presentation", () => {
  assert.equal(patcher.project.amxdtype, 1768515945);
  assert.equal(patcher.openinpresentation, 1);
  assert.equal(patcher.openrect[3], 169);
});

test("only known Max objects are used", () => {
  for (const b of boxes.filter((x) => x.maxclass === "newobj")) {
    assert.ok(MAX_OBJECTS.has(b.text.split(" ")[0]), "unknown object: " + b.text);
  }
});

test("every line joins real boxes on real inlets and outlets", () => {
  for (const { patchline: l } of patcher.lines) {
    const from = byId[l.source[0]];
    const to = byId[l.destination[0]];
    assert.ok(from && to, "line to a missing box");
    assert.ok(l.source[1] < from.numoutlets, `${from.text || from.maxclass} has no outlet ${l.source[1]}`);
    assert.ok(l.destination[1] < to.numinlets, `${to.text || to.maxclass} has no inlet ${l.destination[1]}`);
  }
});

test("the js box has as many outlets as the js declares", () => {
  const js = boxes.find((b) => b.text === `js ${JS_FILE}`);
  assert.equal(js.numoutlets, Number(jsSource.match(/^outlets = (\d+);/m)[1]));
});

test("every message the panel sends has a function in the js", () => {
  assert.ok(MESSAGES.size > 10);
  for (const name of [...MESSAGES, "init"]) {
    assert.match(jsSource, new RegExp(`^function ${name}\\(`, "m"), `no function ${name} in ${JS_FILE}`);
  }
});

test("every scripting name the js looks up is on the panel", () => {
  const varnames = new Set(boxes.map((b) => b.varname).filter(Boolean));
  const looked = new Set([...jsSource.matchAll(/(?:named|show)\("([a-z_]+)"[,)]/g)].map((m) => m[1]));
  for (let i = 0; i < 9; i++) looked.add(`kept_${i}`).add(`lock_${i}`);
  for (let i = 0; i < 3; i++) looked.add(`fav_${i}`);
  for (const name of looked) assert.ok(varnames.has(name), `no box named ${name}`);
});

test("the Kept dials cover the engine's ranges, in its order", () => {
  assert.equal(KEPT.length, KB.PARAMS.length);
  KEPT.forEach((k, i) => {
    const dial = boxes.find((b) => b.varname === `kept_${i}`);
    const v = dial.saved_attribute_attributes.valueof;
    assert.equal(v.parameter_mmin, KB.PARAMS[i].min, k.longname);
    assert.equal(v.parameter_mmax, KB.PARAMS[i].max, k.longname);
    assert.equal(v.parameter_invisible, 0, "automatable");
  });
});

test("parameter names are unique and listed", () => {
  const params = boxes.filter((b) => b.saved_attribute_attributes);
  const names = params.map((b) => b.saved_attribute_attributes.valueof.parameter_longname);
  assert.equal(new Set(names).size, names.length);
  for (const b of params) assert.ok(patcher.parameters[b.id]);
});

test("the panel fits the device", () => {
  for (const b of boxes.filter((x) => x.presentation)) {
    const [x, y, w, h] = b.presentation_rect;
    assert.ok(x >= 0 && y >= 0 && x + w <= patcher.devicewidth && y + h <= 169, `${b.varname || b.text} is off the panel`);
  }
});
