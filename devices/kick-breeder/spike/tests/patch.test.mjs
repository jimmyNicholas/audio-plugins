// Checks on the generated Kick Breeder spike device. Nobody can run Max here, so these read the
// patcher JSON: files in sync, the device type, known objects only, valid lines, the key map,
// and that the panel and the js agree on message names, scripting names and ranges.

import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { createRequire } from "node:module";
import { pack, INSTRUMENT } from "../../../../tools/amxd.mjs";
import { FILE, JS_FILE, PARAM_NAMES, SAVED, PLAY_KEYS, ACTION_KEYS, SLOTS, MESSAGES, patcherText } from "../build-patch.mjs";

const require = createRequire(import.meta.url);
const here = (name) => new URL(name, import.meta.url);
const text = patcherText();
const patcher = JSON.parse(text).patcher;
const boxes = patcher.boxes.map((b) => b.box);
const byId = Object.fromEntries(boxes.map((b) => [b.id, b]));
const byName = Object.fromEntries(boxes.filter((b) => b.varname).map((b) => [b.varname, b]));
const lines = patcher.lines.map((l) => l.patchline);
const jsSource = fs.readFileSync(here(`../${JS_FILE}`), "utf8");
const KB = require(`../${JS_FILE}`);

const targets = (id, outlet) => lines.filter((l) => l.source[0] === id && l.source[1] === outlet).map((l) => byId[l.destination[0]]);
const only = (list, what) => {
  assert.equal(list.length, 1, `expected one ${what}, found ${list.length}`);
  return list[0];
};

// Standard Max 8 and MSP objects only. Check the Max reference before adding to this list.
const MAX_OBJECTS = new Set([
  "midiin", "midiparse", "route", "unpack", "sel", "t", "/", "js", "prepend", "live.thisdevice",
  "deferlow", "line~", "cycle~", "*~", "+~", "noise~", "svf~", "tanh~", "lores~", "clip~", "plugout~",
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
  for (const l of lines) {
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

test("every message the patch sends has a function in the js", () => {
  assert.ok(MESSAGES.size > 15);
  for (const name of MESSAGES) {
    assert.match(jsSource, new RegExp(`^function ${name}\\(`, "m"), `no function ${name} in ${JS_FILE}`);
  }
});

test("every scripting name the js uses is in the patch", () => {
  const looked = new Set([...jsSource.matchAll(/(?:named|show|setBox)\("([a-z_]+)"[,)]/g)].map((m) => m[1]));
  for (let i = 0; i < PARAM_NAMES.length; i++) looked.add(`lock_${i}`);
  for (let i = 0; i < 3; i++) looked.add(`fav_${i}`);
  for (let n = 1; n <= SAVED; n++) {
    looked.add(`saved_${n - 1}`).add(`save_${n}_on`);
    for (let i = 0; i < PARAM_NAMES.length; i++) looked.add(`save_${n}_${i}`);
  }
  for (const slot of SLOTS) for (const part of ["statics", "freq", "noise", "amp", "click"]) looked.add(`${slot}_${part}`);
  for (const name of looked) assert.ok(byName[name], `no box named ${name}`);
});

test("the locks follow the engine's settings, in order", () => {
  assert.deepEqual(PARAM_NAMES, KB.PARAMS.map((p) => p.short));
});

test("the Variation slider covers the engine's range", () => {
  const v = byName.variation.saved_attribute_attributes.valueof;
  assert.equal(byName.variation.maxclass, "live.slider");
  assert.equal(v.parameter_mmin, KB.MIN_STEP);
  assert.equal(v.parameter_mmax, KB.MAX_STEP);
  assert.deepEqual(v.parameter_initial, [KB.START_STEP]);
});

test("saved kicks are hidden, stored-only values", () => {
  const hidden = boxes.filter((b) => /^save_\d_/.test(b.varname || ""));
  assert.equal(hidden.length, SAVED * (PARAM_NAMES.length + 1));
  for (const b of hidden) {
    assert.equal(b.maxclass, "live.numbox");
    assert.ok(!b.presentation);
    assert.equal(b.saved_attribute_attributes.valueof.parameter_invisible, 1);
  }
});

// MIDI route -> sel 0 -> trigger -> that slot's stores.
function slotStores(selBox) {
  const trig = only(targets(selBox.id, 1), "trigger after sel 0");
  assert.equal(trig.text, "t f b b b b b b b");
  return trig;
}

test("the key map plays each slot, and the trigger sets the settings before the envelopes", () => {
  const route = only(boxes.filter((b) => b.text && b.text.startsWith("route 60")), "MIDI route");
  const parse = only(boxes.filter((b) => b.text === "midiparse"), "midiparse");
  assert.ok(targets(parse.id, 0).includes(route));
  Object.entries(PLAY_KEYS).forEach(([slot, note], k) => {
    assert.equal(route.text.split(" ")[k + 1], String(note));
    const sel = only(targets(route.id, k), `sel for ${slot}`);
    const trig = slotStores(sel);
    // Right to left: outlet 6 settings, 5 pitch, 4 noise, 3 amp, 2 click.
    ["statics", "freq", "noise", "amp", "click"].forEach((part, i) => {
      assert.ok(targets(trig.id, 6 - i).includes(byName[`${slot}_${part}`]), `${slot} ${part}`);
    });
    assert.ok(targets(trig.id, 7).some((b) => b.text === `heard_slot ${slot}`));
  });
  assert.deepEqual(PLAY_KEYS, { A: 60, B: 62, C: 64, S1: 65, S2: 67, S3: 69, S4: 71 });
});

test("action keys choose and save, and other notes play the kick last heard", () => {
  const route = boxes.find((b) => b.text && b.text.startsWith("route 60"));
  const playCount = Object.keys(PLAY_KEYS).length;
  Object.entries(ACTION_KEYS).forEach(([message, note], k) => {
    assert.equal(route.text.split(" ")[playCount + k + 1], String(note));
    const sel = only(targets(route.id, playCount + k), `sel for ${message}`);
    assert.equal(only(targets(sel.id, 1), message).text, message);
  });
  assert.deepEqual(ACTION_KEYS, { choose_a: 48, choose_b: 50, choose_c: 52, save_1: 53, save_2: 55, save_3: 57, save_4: 59 });
  const rest = only(targets(route.id, playCount + Object.keys(ACTION_KEYS).length), "unpack for other notes");
  const sel = only(targets(rest.id, 1), "sel for other notes");
  const trig = slotStores(sel);
  assert.ok(targets(trig.id, 6).includes(byName.CUR_statics));
});

test("panel buttons reach the slots through the js", () => {
  const route = only(boxes.filter((b) => b.text === `route ${SLOTS.join(" ")}`), "js route");
  SLOTS.forEach((slot, k) => slotStores(only(targets(route.id, k), `sel for ${slot}`)));
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
