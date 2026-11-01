// PROTOTYPE checks for the Band Spread shift audition. Nobody can run Max here, so these read
// the generated patcher JSON: files in sync, only known Max 8 objects, every line valid, the
// Band filter, both engines behind mute~ and lined up at Lmax, the panel and the readouts.

import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { pack, AUDIO_EFFECT } from "../../../../tools/amxd.mjs";
import { FILE, L_MAX, COPIES, ENGINES, DEVICE_WIDTH, controls, patcherText } from "./build-patch.mjs";

const here = (name) => new URL(name, import.meta.url);
const tapProject = JSON.parse(fs.readFileSync(here("../../../tap/TAP.maxpat"), "utf8")).patcher.project;
const readme = fs.readFileSync(here("README.md"), "utf8");
const maxpatText = fs.readFileSync(here(`${FILE}.maxpat`), "utf8");
const patcher = JSON.parse(maxpatText).patcher;

const boxesOf = (p) => p.boxes.map((b) => b.box);
function allPatchers(p, name = "main") {
  const found = [{ name, patcher: p }];
  for (const b of boxesOf(p)) if (b.patcher) found.push(...allPatchers(b.patcher, b.text));
  return found;
}
const allBoxes = (p) => allPatchers(p).flatMap(({ patcher: q }) => boxesOf(q));
const find = (p, text) => boxesOf(p).filter((b) => b.text === text);
const box = (p, id) => boxesOf(p).find((b) => b.id === id);
function one(p, text) {
  const found = find(p, text);
  assert.equal(found.length, 1, `expected exactly one box "${text}", found ${found.length}`);
  return found[0];
}
const lines = (p) => p.lines.map((l) => l.patchline);
const linesFrom = (p, id, outlet) => lines(p).filter((l) => l.source[0] === id && (outlet === undefined || l.source[1] === outlet));
const targets = (p, id, outlet) => linesFrom(p, id, outlet).map((l) => box(p, l.destination[0]));

// Every object box the device may use: standard Max 8 or MSP objects. The first group was
// loaded in Live by the Slush Pitch Layer audition; the second (filtercoeff~, biquad~,
// freqshift~, clip~) is new here, checked against the Cycling '74 refpages in
// docs/research/max-objects.md, and gets its first real load in this audition.
const MAX_OBJECTS = new Set([
  "plugin~", "plugout~", "live.thisdevice", "delay~", "mute~", "p", "*~", "+~", "line~",
  "selector~", "pitchshift~", "expr", "route", "pak", "change", "sel", "t", "delay", "s", "r",
  "deferlow", "loadmess", "sprintf", "+",
  "filtercoeff~", "biquad~", "freqshift~", "clip~",
]);

test(".maxpat is what build-patch.mjs generates, .amxd is its audio effect pack", () => {
  assert.equal(maxpatText, patcherText(), "run: npm run build");
  assert.ok(Buffer.compare(pack(maxpatText, AUDIO_EFFECT), fs.readFileSync(here(`${FILE}.amxd`))) === 0, "run: npm run build");
});

test("TAP's project block as an audio effect, Defined Latency Lmax, Live's height", () => {
  assert.deepEqual(patcher.project, { ...tapProject, amxdtype: 1633771873 });
  assert.equal(patcher.latency, L_MAX);
  assert.equal(patcher.openinpresentation, 1);
  assert.deepEqual(patcher.openrect, [0, 0, DEVICE_WIDTH, 169]);
  assert.equal(patcher.devicewidth, DEVICE_WIDTH);
});

test("only objects known to exist in Max 8, Australian spelling, no em dashes", () => {
  for (const b of allBoxes(patcher).filter((x) => x.maxclass === "newobj")) {
    const word = b.text.split(" ")[0];
    assert.ok(MAX_OBJECTS.has(word), `${word} is not on the list of known Max 8 objects`);
  }
  for (const text of [maxpatText, readme]) {
    assert.ok(!text.includes("—"));
    assert.ok(!/\b(color|center|behavior|normalize)/i.test(text));
  }
});

test("every patchline joins existing boxes at existing inlets and outlets", () => {
  for (const { name, patcher: p } of allPatchers(patcher)) {
    const ids = new Set();
    for (const b of boxesOf(p)) {
      assert.ok(!ids.has(b.id), `${name}: duplicate id ${b.id}`);
      ids.add(b.id);
    }
    for (const l of lines(p)) {
      const from = box(p, l.source[0]);
      const to = box(p, l.destination[0]);
      assert.ok(from && to, `${name}: line ${JSON.stringify(l)} joins a missing box`);
      assert.ok(l.source[1] < from.numoutlets, `${name}: ${from.text} has no outlet ${l.source[1]}`);
      assert.ok(l.destination[1] < to.numinlets, `${name}: ${to.text} has no inlet ${l.destination[1]}`);
    }
  }
  for (const b of allBoxes(patcher).filter((x) => x.patcher)) {
    const inner = boxesOf(b.patcher);
    assert.equal(b.numinlets, inner.filter((x) => x.maxclass === "inlet").length, b.text);
    assert.equal(b.numoutlets, inner.filter((x) => x.maxclass === "outlet").length, b.text);
  }
});

test("every send has a receive and every receive a send, all local to the device", () => {
  const names = (kind) => new Set(allBoxes(patcher).filter((b) => (b.text || "").startsWith(kind + " ")).map((b) => b.text.split(" ")[1]));
  const sends = names("s");
  const receives = names("r");
  for (const n of [...sends, ...receives]) assert.ok(n.startsWith("---"), `${n} is not device-local`);
  for (const n of sends) assert.ok(receives.has(n), `nothing receives ${n}`);
  for (const n of receives) assert.ok(sends.has(n), `nothing sends ${n}`);
});

test("Band: mono sum into three biquad~ in series, all fed by one filtercoeff~ bandpass, Stages picks one", () => {
  const coeff = one(patcher, "filtercoeff~ bandpass");
  const stages = find(patcher, "biquad~");
  assert.equal(stages.length, 3);
  for (const s of stages) {
    for (let c = 0; c < 5; c++) {
      assert.ok(linesFrom(patcher, coeff.id, c).some((l) => l.destination[0] === s.id && l.destination[1] === c + 1), `coefficient ${c}`);
    }
  }
  assert.equal(targets(patcher, one(patcher, "*~ 0.5").id, 0)[0].id, stages[0].id);
  assert.equal(targets(patcher, stages[0].id, 0).filter((b) => b.id === stages[1].id).length, 1);
  assert.equal(targets(patcher, stages[1].id, 0).filter((b) => b.id === stages[2].id).length, 1);
  const choose = one(patcher, "selector~ 3 1");
  stages.forEach((s, i) => assert.ok(linesFrom(patcher, s.id, 0).some((l) => l.destination[0] === choose.id && l.destination[1] === i + 1)));
});

test("both engines sit behind a mute~, are fed the Band and give one output per Copy", () => {
  const band = one(patcher, "selector~ 3 1");
  for (const name of ["pitch-engine", "freq-engine"]) {
    const sub = one(patcher, `p ${name}`);
    assert.equal(sub.numoutlets, COPIES.length);
    const mute = lines(patcher).find((l) => l.destination[0] === sub.id && l.destination[1] === 0);
    assert.equal(box(patcher, mute.source[0]).text, "mute~");
    assert.ok(linesFrom(patcher, band.id, 0).some((l) => l.destination[0] === sub.id && l.destination[1] === 1));
  }
});

test("pitch engine: a pitchshift~ per Copy, each compensated to Lmax by its measured latency", () => {
  const p = one(patcher, "p pitch-engine").patcher;
  const shifters = find(p, "pitchshift~ @quality good @reportlatency 1");
  assert.equal(shifters.length, COPIES.length);
  for (const ps of shifters) {
    const [delay] = targets(p, ps.id, 0);
    assert.equal(delay.text, `delay~ ${L_MAX * 2} ${L_MAX}`);
    assert.equal(targets(p, ps.id, 1)[0].text, "route latency");
  }
  assert.equal(find(p, `expr max(${L_MAX} - $i1\\, 0)`).length, COPIES.length);
});

test("frequency engine: a freqshift~ per Copy, left (signed) outlet, delayed by Lmax", () => {
  const p = one(patcher, "p freq-engine").patcher;
  const shifters = find(p, "freqshift~");
  assert.equal(shifters.length, COPIES.length);
  for (const s of shifters) {
    assert.equal(linesFrom(p, s.id, 1).length, 0, "the negative sideband outlet is not used");
    assert.equal(targets(p, s.id, 0)[0].text, `delay~ ${L_MAX * 2} ${L_MAX}`);
  }
  COPIES.forEach((_, k) => one(p, `r ---c${k + 1}_fhz`));
});

test("engine switching: Pitch on for tab 0, the frequency engine for tabs 1 and 2", () => {
  one(patcher, "expr $i1 == 0");
  one(patcher, "expr $i1 > 0");
  assert.equal(find(patcher, "expr ($i1 == 2) * $f4 + ($i1 != 2) * $f2 * (pow(2.\\, $f3 / 12.) - 1.)").length, COPIES.length);
});

test("output goes through Gain and clip~ to both sides", () => {
  const clip = one(patcher, "clip~ -1. 1.");
  const plugout = one(patcher, "plugout~");
  assert.deepEqual(linesFrom(patcher, clip.id, 0).map((l) => l.destination).sort(), [[plugout.id, 0], [plugout.id, 1]]);
});

test("panel has exactly the expected controls, all on the panel, each sending its value", () => {
  const found = boxesOf(patcher).filter((b) => b.saved_attribute_attributes);
  const expected = controls();
  assert.deepEqual(found.map((b) => b.saved_attribute_attributes.valueof.parameter_longname).sort(), expected.map((c) => c.longname).sort());
  assert.deepEqual(expected.find((c) => c.longname === "Engine").items, ENGINES);
  for (const b of found) {
    const v = b.saved_attribute_attributes.valueof;
    const c = expected.find((x) => x.longname === v.parameter_longname);
    assert.deepEqual(v.parameter_initial, [c.initial], c.longname);
    assert.deepEqual(patcher.parameters[b.id], [c.longname, c.shortname, 0]);
    assert.ok(targets(patcher, b.id, 0).some((t) => t.text === `s ---${c.send}`), `${c.longname} sends nothing`);
  }
});

test("no two panel items overlap, and all are inside the device", () => {
  const shown = boxesOf(patcher).filter((b) => b.presentation);
  for (const b of shown) {
    const r = b.presentation_rect;
    assert.ok(r[0] >= 0 && r[1] >= 0 && r[0] + r[2] <= DEVICE_WIDTH && r[1] + r[3] <= 169, `${b.text || b.varname} is off the panel`);
  }
  for (let i = 0; i < shown.length; i++) {
    for (let j = i + 1; j < shown.length; j++) {
      const [a, c] = [shown[i].presentation_rect, shown[j].presentation_rect];
      const overlap = a[0] < c[0] + c[2] && c[0] < a[0] + a[2] && a[1] < c[1] + c[3] && c[1] < a[1] + a[3];
      assert.ok(!overlap, `${shown[i].text || shown[i].varname} overlaps ${shown[j].text || shown[j].varname}`);
    }
  }
});
