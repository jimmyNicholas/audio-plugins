// Checks on the Slush Pitch Layer Audition patch. Nobody can run Max here, so these read the
// generated patcher JSON and check it against SPEC.md: files in sync, the project block, the
// dry path, every switch, the mutes, the latency compensation and the delay line shifter.

import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { pack, AUDIO_EFFECT } from "../../../../tools/amxd.mjs";
import { buildPatcher, L_MAX } from "./build-patch.mjs";

const here = (name) => new URL(name, import.meta.url);
const maxpatText = fs.readFileSync(here("Slush Pitch Layer Audition.maxpat"), "utf8");
const patcher = JSON.parse(maxpatText).patcher;

// ---- helpers ------------------------------------------------------------------------------

function boxesOf(p) {
  return p.boxes.map((b) => b.box);
}

function allPatchers(p, name = "main") {
  const found = [{ name, patcher: p }];
  for (const box of boxesOf(p)) {
    if (box.patcher) found.push(...allPatchers(box.patcher, box.text));
  }
  return found;
}

function allBoxes(p) {
  return allPatchers(p).flatMap(({ patcher: q }) => boxesOf(q));
}

function find(p, text) {
  return boxesOf(p).filter((b) => b.text === text);
}

function one(p, text) {
  const found = find(p, text);
  assert.equal(found.length, 1, `expected exactly one box "${text}", found ${found.length}`);
  return found[0];
}

function linesFrom(p, id, outlet) {
  return p.lines
    .map((l) => l.patchline)
    .filter((l) => l.source[0] === id && (outlet === undefined || l.source[1] === outlet));
}

function linesTo(p, id, inlet) {
  return p.lines
    .map((l) => l.patchline)
    .filter((l) => l.destination[0] === id && (inlet === undefined || l.destination[1] === inlet));
}

function box(p, id) {
  return boxesOf(p).find((b) => b.id === id);
}

function subpatcher(name) {
  const b = one(patcher, `p ${name}`);
  return { box: b, patcher: b.patcher };
}

function parameter(longname) {
  const found = boxesOf(patcher).filter(
    (b) => b.saved_attribute_attributes && b.saved_attribute_attributes.valueof.parameter_longname === longname,
  );
  assert.equal(found.length, 1, `expected one parameter "${longname}"`);
  return found[0];
}

// ---- files in sync -----------------------------------------------------------------------

test("the committed .maxpat is what build-patch.mjs generates", () => {
  assert.equal(maxpatText, JSON.stringify(buildPatcher(), null, 2) + "\n", "run: npm run build --prefix devices/slush-wave/prototype/pitch-layer-audition");
});

test("the committed .amxd is pack of the .maxpat as an audio effect", () => {
  const device = fs.readFileSync(here("Slush Pitch Layer Audition.amxd"));
  assert.ok(Buffer.compare(pack(maxpatText, AUDIO_EFFECT), device) === 0, "run: npm run build --prefix devices/slush-wave/prototype/pitch-layer-audition");
});

// ---- device settings ---------------------------------------------------------------------

test("project block is TAP's with only amxdtype changed to audio effect", () => {
  const tap = JSON.parse(fs.readFileSync(here("../../../tap/TAP.maxpat"), "utf8")).patcher.project;
  assert.deepEqual(patcher.project, { ...tap, amxdtype: 1633771873 });
});

test("Defined Latency is Lmax, MPE is off, the device opens at Live's height", () => {
  assert.equal(L_MAX, 8192);
  assert.equal(patcher.latency, 8192);
  assert.equal(patcher.is_mpe, 0);
  assert.equal(patcher.openinpresentation, 1);
  assert.equal(patcher.openrect[3], 169);
});

test("plain MSP and live.* only: no gen~, js, pfft~ or packages", () => {
  for (const b of allBoxes(patcher)) {
    const name = (b.text || "").split(" ")[0];
    assert.ok(!["gen~", "js", "jsui", "pfft~", "mc.gen~", "bpatcher"].includes(name), `found ${name}`);
    assert.ok(!["jsui", "bpatcher"].includes(b.maxclass), `found ${b.maxclass}`);
  }
});

test("Australian spelling, no em dashes anywhere in the patch or README", () => {
  const readme = fs.readFileSync(here("README.md"), "utf8");
  for (const [name, text] of [["patch", maxpatText], ["README", readme]]) {
    assert.ok(!text.includes("—"), `${name} has an em dash`);
    assert.ok(!/\b(color|center|behavior|normalize)/i.test(text), `${name} has US spelling`);
  }
});

// ---- wiring is well formed ---------------------------------------------------------------

test("every patchline joins existing boxes at existing inlets and outlets", () => {
  for (const { name, patcher: p } of allPatchers(patcher)) {
    const ids = new Set();
    for (const b of boxesOf(p)) {
      assert.ok(!ids.has(b.id), `${name}: duplicate id ${b.id}`);
      ids.add(b.id);
    }
    for (const { patchline: l } of p.lines) {
      const from = box(p, l.source[0]);
      const to = box(p, l.destination[0]);
      assert.ok(from && to, `${name}: line ${JSON.stringify(l)} joins a missing box`);
      assert.ok(l.source[1] < from.numoutlets, `${name}: ${from.text} has no outlet ${l.source[1]}`);
      assert.ok(l.destination[1] < to.numinlets, `${name}: ${to.text} has no inlet ${l.destination[1]}`);
    }
  }
});

test("each subpatcher box has as many inlets and outlets as its contents", () => {
  for (const b of allBoxes(patcher).filter((x) => x.patcher)) {
    const inner = boxesOf(b.patcher);
    assert.equal(b.numinlets, inner.filter((x) => x.maxclass === "inlet").length, b.text);
    assert.equal(b.numoutlets, inner.filter((x) => x.maxclass === "outlet").length, b.text);
  }
});

test("every send has a receive and every receive a send, all local to the device", () => {
  const names = (kind) =>
    new Set(allBoxes(patcher).filter((b) => (b.text || "").startsWith(kind + " ")).map((b) => b.text.split(" ")[1]));
  const sends = names("s");
  const receives = names("r");
  for (const n of [...sends, ...receives]) assert.ok(n.startsWith("---"), `${n} is not device-local`);
  for (const n of sends) assert.ok(receives.has(n), `nothing receives ${n}`);
  for (const n of receives) assert.ok(sends.has(n), `nothing sends ${n}`);
});

// ---- signal flow -------------------------------------------------------------------------

test("dry path: each input channel goes through delay~ at exactly Lmax to plugout~", () => {
  const plugin = one(patcher, "plugin~");
  const plugout = one(patcher, "plugout~");
  for (const channel of [0, 1]) {
    const delays = linesFrom(patcher, plugin.id, channel)
      .map((l) => box(patcher, l.destination[0]))
      .filter((b) => b.text === "delay~ 16384 8192");
    assert.equal(delays.length, 1, `channel ${channel}`);
    const gain = box(patcher, linesFrom(patcher, delays[0].id, 0)[0].destination[0]);
    assert.equal(gain.text, "*~ 0.");
    assert.deepEqual(linesFrom(patcher, gain.id, 0)[0].destination, [plugout.id, channel]);
  }
});

const VERSIONS = ["doubles-delayline", "doubles-pitchshift", "interval-pitchshift", "interval-delayline"];

test("each version is an embedded subpatcher with a mute~ in front, fed both channels, out to plugout~", () => {
  const plugin = one(patcher, "plugin~");
  const plugout = one(patcher, "plugout~");
  for (const name of VERSIONS) {
    const { box: p } = subpatcher(name);
    const into = linesTo(patcher, p.id, 0).map((l) => box(patcher, l.source[0]));
    assert.deepEqual(into.map((b) => b.text), ["mute~"], name);
    assert.deepEqual(linesTo(patcher, p.id, 1).map((l) => l.source), [[plugin.id, 0]], name);
    assert.deepEqual(linesTo(patcher, p.id, 2).map((l) => l.source), [[plugin.id, 1]], name);
    for (const channel of [0, 1]) {
      const gain = box(patcher, linesFrom(patcher, p.id, channel)[0].destination[0]);
      assert.equal(gain.text, "*~ 0.", name);
      assert.deepEqual(linesFrom(patcher, gain.id, 0)[0].destination, [plugout.id, channel], name);
    }
  }
});

test("delay line versions are delayed by Lmax on both channels", () => {
  for (const name of ["doubles-delayline", "interval-delayline"]) {
    const { patcher: p } = subpatcher(name);
    assert.equal(find(p, "delay~ 16384 8192").length, 2, name);
  }
});

test("every pitchshift~ is compensated: its measured latency sets its own delay~ to Lmax - L", () => {
  for (const name of ["doubles-pitchshift", "interval-pitchshift"]) {
    const { patcher: p } = subpatcher(name);
    const shifters = boxesOf(p).filter((b) => (b.text || "").startsWith("pitchshift~"));
    assert.equal(shifters.length, 2, name);
    for (const ps of shifters) {
      assert.match(ps.text, /@quality good/);
      const [audio] = linesFrom(p, ps.id, 0);
      const delay = box(p, audio.destination[0]);
      assert.equal(delay.text, "delay~ 16384 8192", name);
      const route = box(p, linesFrom(p, ps.id, 1)[0].destination[0]);
      assert.equal(route.text, "route latency");
      const trigger = box(p, linesFrom(p, route.id, 0)[0].destination[0]);
      const compensation = linesFrom(p, trigger.id)
        .map((l) => box(p, l.destination[0]))
        .find((b) => b.text === "expr max(8192 - $i1\\, 0)");
      assert.ok(compensation, `${name}: no compensation expr`);
      assert.deepEqual(linesFrom(p, compensation.id, 0).map((l) => l.destination), [[delay.id, 1]]);
    }
    assert.ok(find(p, "r ---measure").length === 1, `${name} never measures`);
  }
});

test("measured latencies are printed to the Max Console in the spec's format", () => {
  const prints = (name) => {
    const { patcher: p } = subpatcher(name);
    return boxesOf(p)
      .filter((b) => b.maxclass === "message" && linesFrom(p, b.id, 0).some((l) => box(p, l.destination[0]).text === "print audition"))
      .map((b) => b.text);
  };
  assert.ok(prints("doubles-pitchshift").includes("pitchshift doubles $2 latency $1 samples"));
  assert.ok(prints("interval-pitchshift").includes("pitchshift interval $2 latency $1 samples"));
});

for (const name of ["doubles-pitchshift", "interval-pitchshift"]) {
  test(`Quality reaches both ${name} pitchshift~ objects and remeasures`, () => {
    const { patcher: p } = subpatcher(name);
    one(p, "r ---i_quality");
    for (const q of ["basic", "good", "better", "best"]) {
      const msg = boxesOf(p).find((b) => b.text === `quality ${q}`);
      assert.ok(msg, q);
      const targets = linesFrom(p, msg.id, 0).map((l) => box(p, l.destination[0]).text);
      assert.equal(targets.filter((t) => t.startsWith("pitchshift~")).length, 2, q);
    }
    assert.ok(boxesOf(p).filter((b) => b.text === "getlatency").length >= 2);
  });
}

test("switching a version on measures latency again, once its fade-in starts", () => {
  const delays = find(patcher, "delay 250");
  assert.equal(delays.length, 4, "one fade-in delay per version");
  for (const d of delays) {
    const targets = linesFrom(patcher, d.id, 0).map((l) => box(patcher, l.destination[0]).text);
    assert.ok(targets.includes("s ---measure"), "fade-in does not remeasure");
  }
});

// ---- the delay line shifter ---------------------------------------------------------------

test("the delay line shifter is two Hann-windowed taps on a 200 ms tapin~, swept by a phasor~", () => {
  const shifters = allPatchers(patcher).filter((x) => x.name === "p dlshift");
  assert.equal(shifters.length, 4, "two per delay line version");
  for (const { patcher: p } of shifters) {
    one(p, "tapin~ 200");
    one(p, "tapout~ 3 3");
    const rate = one(p, "expr~ (1. - pow(2.\\, $v1 / 1200.)) * 1000. / max($f2\\, 1.)");
    assert.equal(box(p, linesFrom(p, rate.id, 0)[0].destination[0]).text, "phasor~");
    one(p, "%~ 1.");
    assert.equal(find(p, "cos~").length, 2);
    assert.equal(find(p, "+~ 3.").length, 2, "base delay of 3 ms on each tap");
    assert.equal(boxesOf(p).filter((b) => b.maxclass === "inlet").length, 3);
  }
});

test("doubles shifters get a fixed 30 ms window; interval shifters get the Window dial", () => {
  const { patcher: d } = subpatcher("doubles-delayline");
  one(d, "loadmess 30.");
  const { patcher: i } = subpatcher("interval-delayline");
  one(i, "r ---i_window");
});

test("each double has its own rand~, in both doubles engines", () => {
  for (const name of ["doubles-delayline", "doubles-pitchshift"]) {
    const { patcher: p } = subpatcher(name);
    assert.equal(boxesOf(p).filter((b) => (b.text || "").startsWith("rand~")).length, 2, name);
    one(p, "r ---drift_rate");
    one(p, "r ---drift_amt");
  }
  const { patcher: ps } = subpatcher("doubles-pitchshift");
  assert.equal(find(ps, "snapshot~ 20").length, 2, "pitchshift drift steps every 20 ms");
});

// ---- panel ---------------------------------------------------------------------------------

const CONTROLS = [
  // longname, maxclass, min, max, initial, enum
  ["Layers", "live.text", 0, 1, 1],
  ["Solo", "live.tab", 0, 2, 0, ["Off", "Doubles", "Interval"]],
  ["D Engine", "live.tab", 0, 1, 0, ["Delay line", "Pitchshift"]],
  ["D Source", "live.tab", 0, 1, 0, ["Per channel", "Mono sum"]],
  ["D Cents", "live.dial", 0, 30, 12],
  ["D Level", "live.dial", -70, 0, -9],
  ["D Drift", "live.text", 0, 1, 0],
  ["D Drift Depth", "live.dial", 0, 10, 3],
  ["D Drift Rate", "live.dial", 0.05, 1, 0.2],
  ["D Trim DL", "live.dial", -12, 12, 0],
  ["D Trim PS", "live.dial", -12, 12, 0],
  ["I Engine", "live.tab", 0, 1, 0, ["Pitchshift", "Delay line"]],
  ["I Quality", "live.menu", 0, 3, 1, ["basic", "good", "better", "best"]],
  ["I Window", "live.dial", 20, 150, 60],
  ["I Interval", "live.tab", 0, 1, 0, ["Octave down", "Fifth down"]],
  ["I Stereo", "live.tab", 0, 1, 0, ["Mono centre", "Per channel"]],
  ["I Level", "live.dial", -70, 0, -12],
  ["I Trim PS", "live.dial", -12, 12, 0],
  ["I Trim DL", "live.dial", -12, 12, 0],
];

for (const [longname, maxclass, min, max, initial, enums] of CONTROLS) {
  test(`panel control ${longname}`, () => {
    const b = parameter(longname);
    const v = b.saved_attribute_attributes.valueof;
    assert.equal(b.maxclass, maxclass);
    assert.equal(b.presentation, 1);
    assert.equal(v.parameter_mmin, min);
    assert.equal(v.parameter_mmax, max);
    assert.deepEqual(v.parameter_initial, [initial]);
    assert.equal(v.parameter_initial_enable, 1);
    if (enums) assert.deepEqual(v.parameter_enum, enums);
    assert.deepEqual(patcher.parameters[b.id].slice(0, 1), [longname]);
    const r = b.presentation_rect;
    assert.ok(r[0] >= 0 && r[0] + r[2] <= patcher.devicewidth && r[1] >= 0 && r[1] + r[3] <= 169, `${longname} is off the panel`);
  });
}

test("there are no parameters beyond the spec's", () => {
  const all = boxesOf(patcher).filter((b) => b.saved_attribute_attributes);
  assert.deepEqual(all.map((b) => b.saved_attribute_attributes.valueof.parameter_longname).sort(), CONTROLS.map((c) => c[0]).sort());
});

test("dB dials use dB units and the cents dial steps by 1", () => {
  for (const n of ["D Level", "I Level", "D Trim DL", "D Trim PS", "I Trim PS", "I Trim DL"]) {
    assert.equal(parameter(n).saved_attribute_attributes.valueof.parameter_unitstyle, 4, n);
  }
  assert.equal(parameter("D Cents").saved_attribute_attributes.valueof.parameter_type, 1);
});

test("readouts: Defined Latency text, both pitchshift latencies, and a warning line", () => {
  const comments = boxesOf(patcher).filter((b) => b.presentation && (b.maxclass === "comment" || b.maxclass === "live.comment"));
  assert.ok(comments.some((b) => b.text === "Defined Latency: 8192 smp"));
  for (const v of ["d_readout", "i_readout", "warning"]) {
    const c = boxesOf(patcher).find((b) => b.varname === v);
    assert.ok(c && c.presentation === 1, v);
    assert.ok(linesTo(patcher, c.id, 0).length > 0, `${v} is never set`);
  }
});

// ---- control logic --------------------------------------------------------------------------

test("the switching logic covers every version and the dry path", () => {
  const exprs = boxesOf(patcher).map((b) => b.text || "");
  assert.ok(exprs.includes("expr $i1 == 0 || $i2 == 0"), "dry: on unless Solo is on with layers on");
  assert.ok(exprs.includes("expr $i1 && $i2 != 2 && $f3 > -69.9 && $i4 == 0"), "doubles delay line");
  assert.ok(exprs.includes("expr $i1 && $i2 != 2 && $f3 > -69.9 && $i4 == 1"), "doubles pitchshift");
  assert.ok(exprs.includes("expr $i1 && $i2 != 1 && $f3 > -69.9 && $i4 == 0"), "interval pitchshift");
  assert.ok(exprs.includes("expr $i1 && $i2 != 1 && $f3 > -69.9 && $i4 == 1"), "interval delay line");
});
