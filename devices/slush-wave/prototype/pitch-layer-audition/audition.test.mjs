// Checks on the Slush Pitch Layer audition devices. Nobody can run Max here, so these read the
// generated patcher JSON and check it against SPEC.md: files in sync, the project block, the
// dry path, every switch, the mutes, the latency compensation and the shifters, per device.

import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import { pack, AUDIO_EFFECT } from "../../../../tools/amxd.mjs";
import { DEVICES, L_MAX, patcherText } from "./build-patch.mjs";

const here = (name) => new URL(name, import.meta.url);
const tapProject = JSON.parse(fs.readFileSync(here("../../../tap/TAP.maxpat"), "utf8")).patcher.project;
const readme = fs.readFileSync(here("README.md"), "utf8");

// ---- helpers ------------------------------------------------------------------------------

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
const linesTo = (p, id, inlet) => lines(p).filter((l) => l.destination[0] === id && (inlet === undefined || l.destination[1] === inlet));
const targets = (p, id, outlet) => linesFrom(p, id, outlet).map((l) => box(p, l.destination[0]));

const parameters = (p) => boxesOf(p).filter((b) => b.saved_attribute_attributes);
const longname = (b) => b.saved_attribute_attributes.valueof.parameter_longname;

// Every object box the devices may use. Each is a standard Max 8 or MSP object; anything new
// must be checked against the Max reference before it goes on this list. (expr~ was not: it is
// Pure Data, and Max reported "expr~: No such object".)
const MAX_OBJECTS = new Set([
  "plugin~", "plugout~", "live.thisdevice", "delay~", "mute~", "p", "*~", "+~", "%~", "line~",
  "phasor~", "cos~", "tapin~", "tapout~", "selector~", "rand~", "snapshot~", "pitchshift~",
  "dspstate~", "expr", "route", "pak", "pack", "change", "sel", "t", "delay", "s", "r", "print",
  "deferlow", "loadmess", "sprintf", "+",
]);

// ---- what each device should have ---------------------------------------------------------

const DB = 4;
const CONTROL = {
  Layers: ["live.text", 0, 1, 1],
  "PS Quality": ["live.menu", 0, 3, 1, ["basic", "good", "better", "best"]],
  "D Source": ["live.tab", 0, 1, 0, ["Per channel", "Mono sum"]],
  "D Cents": ["live.dial", 0, 30, 12],
  "D Level": ["live.dial", -70, 0, -9, null, DB],
  "D Vib Rate": ["live.dial", 0.1, 5, 0.67],
  "D Drift": ["live.text", 0, 1, 0],
  "D Drift Depth": ["live.dial", 0, 10, 3],
  "D Drift Rate": ["live.dial", 0.05, 1, 0.2],
  "D Trim DL": ["live.dial", -12, 12, 0, null, DB],
  "D Trim PS": ["live.dial", -12, 12, 0, null, DB],
  "D Trim VB": ["live.dial", -12, 12, 0, null, DB],
  "I Interval": ["live.tab", 0, 1, 0, ["Octave down", "Fifth down"]],
  "I Stereo": ["live.tab", 0, 1, 0, ["Mono centre", "Per channel"]],
  "I Window": ["live.dial", 20, 150, 60],
  "I Level": ["live.dial", -70, 0, -12, null, DB],
  "I Trim PS": ["live.dial", -12, 12, 0, null, DB],
  "I Trim DL": ["live.dial", -12, 12, 0, null, DB],
};

const choices = (items) => ["live.tab", 0, items.length - 1, 0, items];

const EXPECT = {
  "Audition 1 Alignment": { versions: [], controls: {} },
  "Audition 2 Doubles Delay Line": {
    versions: ["doubles-delayline", "doubles-vibrato"],
    controls: {
      Solo: choices(["Off", "Doubles"]),
      "D Engine": choices(["Delay line", "Vibrato"]),
      ...pick("Layers", "D Source", "D Cents", "D Level", "D Vib Rate", "D Trim DL", "D Trim VB"),
    },
  },
  "Audition 3 Doubles Pitchshift": {
    versions: ["doubles-pitchshift"],
    controls: { Solo: choices(["Off", "Doubles"]), ...pick("Layers", "PS Quality", "D Source", "D Cents", "D Level") },
  },
  "Audition 4 Doubles Drift": {
    versions: ["doubles-delayline", "doubles-pitchshift"],
    drift: true,
    controls: {
      Solo: choices(["Off", "Doubles"]),
      "D Engine": choices(["Delay line", "Pitchshift"]),
      ...pick("Layers", "PS Quality", "D Source", "D Cents", "D Level", "D Trim DL", "D Trim PS", "D Drift", "D Drift Depth", "D Drift Rate"),
    },
  },
  "Audition 5 Interval Pitchshift": {
    versions: ["interval-pitchshift"],
    controls: { Solo: choices(["Off", "Interval"]), ...pick("Layers", "PS Quality", "I Interval", "I Stereo", "I Level") },
  },
  "Audition 6 Interval Delay Line": {
    versions: ["interval-delayline"],
    controls: { Solo: choices(["Off", "Interval"]), ...pick("Layers", "I Interval", "I Stereo", "I Window", "I Level") },
  },
  "Audition 7 Combined": {
    versions: ["doubles-delayline", "doubles-pitchshift", "doubles-vibrato", "interval-pitchshift", "interval-delayline"],
    drift: true,
    controls: {
      Solo: choices(["Off", "Doubles", "Interval"]),
      "D Engine": choices(["Delay line", "Pitchshift", "Vibrato"]),
      "I Engine": choices(["Pitchshift", "Delay line"]),
      ...pick(...Object.keys(CONTROL)),
    },
  },
};

function pick(...names) {
  return Object.fromEntries(names.map((n) => [n, CONTROL[n]]));
}

test("the generator makes exactly the expected devices", () => {
  assert.deepEqual(DEVICES.map((d) => d.file), Object.keys(EXPECT));
});

test("README has a section for every device", () => {
  for (const d of DEVICES) assert.ok(readme.includes(`**${d.file}.amxd**`), d.file);
});

test("Australian spelling, no em dashes in the README", () => {
  assert.ok(!readme.includes("—"));
  assert.ok(!/\b(color|center|behavior|normalize)/i.test(readme));
});

// ---- per device ----------------------------------------------------------------------------

for (const device of DEVICES) {
  const name = device.file;
  const expect = EXPECT[name];
  const maxpatPath = here(`${name}.maxpat`);
  const maxpatText = fs.existsSync(maxpatPath) ? fs.readFileSync(maxpatPath, "utf8") : "";
  const patcher = maxpatText ? JSON.parse(maxpatText).patcher : { boxes: [], lines: [] };
  const sub = (v) => one(patcher, `p ${v}`);

  test(`${name}: .maxpat is what build-patch.mjs generates, .amxd is its audio effect pack`, () => {
    assert.equal(maxpatText, patcherText(device), "run: npm run build --prefix devices/slush-wave/prototype/pitch-layer-audition");
    const amxd = fs.readFileSync(here(`${name}.amxd`));
    assert.ok(Buffer.compare(pack(maxpatText, AUDIO_EFFECT), amxd) === 0, "run: npm run build");
  });

  test(`${name}: TAP's project block as an audio effect, Defined Latency Lmax, Live's height`, () => {
    assert.deepEqual(patcher.project, { ...tapProject, amxdtype: 1633771873 });
    assert.equal(patcher.latency, L_MAX);
    assert.equal(L_MAX, 8192);
    assert.equal(patcher.is_mpe, 0);
    assert.equal(patcher.openinpresentation, 1);
    assert.equal(patcher.openrect[3], 169);
    assert.equal(patcher.openrect[2], patcher.devicewidth);
  });

  test(`${name}: only objects known to exist in Max 8 (no gen~, js, or Pure Data names like expr~)`, () => {
    for (const b of allBoxes(patcher).filter((x) => x.maxclass === "newobj")) {
      const word = b.text.split(" ")[0];
      assert.ok(MAX_OBJECTS.has(word), `${word} is not on the list of known Max 8 objects`);
    }
    assert.ok(!maxpatText.includes("—"));
    assert.ok(!/\b(color|center|behavior|normalize)/i.test(maxpatText));
  });

  test(`${name}: every patchline joins existing boxes at existing inlets and outlets`, () => {
    for (const { name: where, patcher: p } of allPatchers(patcher)) {
      const ids = new Set();
      for (const b of boxesOf(p)) {
        assert.ok(!ids.has(b.id), `${where}: duplicate id ${b.id}`);
        ids.add(b.id);
      }
      for (const l of lines(p)) {
        const from = box(p, l.source[0]);
        const to = box(p, l.destination[0]);
        assert.ok(from && to, `${where}: line ${JSON.stringify(l)} joins a missing box`);
        assert.ok(l.source[1] < from.numoutlets, `${where}: ${from.text} has no outlet ${l.source[1]}`);
        assert.ok(l.destination[1] < to.numinlets, `${where}: ${to.text} has no inlet ${l.destination[1]}`);
      }
    }
    for (const b of allBoxes(patcher).filter((x) => x.patcher)) {
      const inner = boxesOf(b.patcher);
      assert.equal(b.numinlets, inner.filter((x) => x.maxclass === "inlet").length, b.text);
      assert.equal(b.numoutlets, inner.filter((x) => x.maxclass === "outlet").length, b.text);
    }
  });

  test(`${name}: every send has a receive and every receive a send, all local to the device`, () => {
    const names = (kind) => new Set(allBoxes(patcher).filter((b) => (b.text || "").startsWith(kind + " ")).map((b) => b.text.split(" ")[1]));
    const sends = names("s");
    const receives = names("r");
    for (const n of [...sends, ...receives]) assert.ok(n.startsWith("---"), `${n} is not device-local`);
    for (const n of sends) assert.ok(receives.has(n), `nothing receives ${n}`);
    for (const n of receives) assert.ok(sends.has(n), `nothing sends ${n}`);
  });

  test(`${name}: dry path is delay~ at exactly Lmax into a gain into plugout~, started on load`, () => {
    const plugin = one(patcher, "plugin~");
    const plugout = one(patcher, "plugout~");
    for (const channel of [0, 1]) {
      const delays = targets(patcher, plugin.id, channel).filter((b) => b.text === "delay~ 16384 8192");
      assert.equal(delays.length, 1, `channel ${channel}`);
      const [gain] = targets(patcher, delays[0].id, 0);
      assert.equal(gain.text, "*~ 0.");
      assert.deepEqual(linesFrom(patcher, gain.id, 0)[0].destination, [plugout.id, channel]);
    }
    const load = targets(patcher, one(patcher, "live.thisdevice").id, 0)[0];
    assert.ok(targets(patcher, load.id).some((b) => b.text === "pak 1 0"), "dry gain is never started");
    one(patcher, "expr $i1 == 0 || $i2 == 0");
  });

  test(`${name}: has exactly its versions, each behind a mute~, fed both channels, out to plugout~`, () => {
    const plugin = one(patcher, "plugin~");
    const plugout = one(patcher, "plugout~");
    const present = boxesOf(patcher).filter((b) => b.patcher).map((b) => b.text.slice(2));
    assert.deepEqual(present, expect.versions);
    for (const v of expect.versions) {
      const p = sub(v);
      assert.deepEqual(linesTo(patcher, p.id, 0).map((l) => box(patcher, l.source[0]).text), ["mute~"], v);
      assert.deepEqual(linesTo(patcher, p.id, 1).map((l) => l.source), [[plugin.id, 0]], v);
      assert.deepEqual(linesTo(patcher, p.id, 2).map((l) => l.source), [[plugin.id, 1]], v);
      for (const channel of [0, 1]) {
        const [gain] = targets(patcher, p.id, channel);
        assert.equal(gain.text, "*~ 0.", v);
        assert.deepEqual(linesFrom(patcher, gain.id, 0)[0].destination, [plugout.id, channel], v);
      }
    }
  });

  test(`${name}: switching logic, one on-expr per version`, () => {
    const layers = ["doubles", "interval"].filter((l) => expect.versions.some((v) => v.startsWith(l)));
    for (const v of expect.versions) {
      const layer = layers.indexOf(v.split("-")[0]) + 1;
      const engine = expect.versions.filter((x) => x.startsWith(v.split("-")[0])).indexOf(v);
      one(patcher, `expr $i1 && ($i2 == 0 || $i2 == ${layer}) && $f3 > -69.9 && $i4 == ${engine}`);
    }
    assert.equal(find(patcher, "delay 250").length, expect.versions.length, "one fade-in delay per version");
  });

  const pitchshifts = expect.versions.filter((v) => v.endsWith("pitchshift"));
  if (pitchshifts.length) {
    test(`${name}: switching a version on measures latency again`, () => {
      for (const d of find(patcher, "delay 250")) {
        assert.ok(targets(patcher, d.id, 0).some((b) => b.text === "s ---measure"), "fade-in does not remeasure");
      }
    });
  }

  for (const v of pitchshifts) {
    const layer = v.split("-")[0];
    test(`${name}: ${v} compensates each pitchshift~ by Lmax - L, prints it, follows Quality`, () => {
      const p = sub(v).patcher;
      const shifters = boxesOf(p).filter((b) => (b.text || "").startsWith("pitchshift~"));
      assert.equal(shifters.length, 2);
      for (const ps of shifters) {
        assert.match(ps.text, /@quality good/);
        const [delay] = targets(p, ps.id, 0);
        assert.equal(delay.text, "delay~ 16384 8192");
        const [route] = targets(p, ps.id, 1);
        assert.equal(route.text, "route latency");
        const [trigger] = targets(p, route.id, 0);
        const compensation = targets(p, trigger.id).find((b) => b.text === "expr max(8192 - $i1\\, 0)");
        assert.ok(compensation, "no compensation expr");
        assert.deepEqual(linesFrom(p, compensation.id, 0).map((l) => l.destination), [[delay.id, 1]]);
      }
      const printed = boxesOf(p).filter((b) => b.maxclass === "message" && targets(p, b.id, 0).some((t) => t.text === "print audition"));
      assert.deepEqual(printed.map((b) => b.text), [`pitchshift ${layer} $2 latency $1 samples`]);
      one(p, "r ---quality");
      one(p, "r ---measure");
      for (const q of ["basic", "good", "better", "best"]) {
        const msg = one(p, `quality ${q}`);
        assert.equal(targets(p, msg.id, 0).filter((t) => t.text.startsWith("pitchshift~")).length, 2, q);
      }
      assert.equal(find(p, "getlatency").length, 2);
    });
  }

  for (const v of expect.versions.filter((x) => x.endsWith("delayline"))) {
    test(`${name}: ${v} uses two delay line shifters, delayed by Lmax`, () => {
      const p = sub(v).patcher;
      assert.equal(find(p, "p dlshift").length, 2);
      assert.equal(find(p, "delay~ 16384 8192").length, 2);
      if (v === "doubles-delayline") one(p, "loadmess 30.");
      else one(p, "r ---i_window");
    });
  }

  if (expect.versions.includes("doubles-vibrato")) {
    test(`${name}: vibrato sweeps a 30 ms delay per side, sized so the peak detune is Cents`, () => {
      const p = sub("doubles-vibrato").patcher;
      one(p, "expr min((pow(2.\\, $f1 / 1200.) - 1.) * 1000. / (6.283185 * max($f2\\, 0.01))\\, 25.)");
      assert.equal(find(p, "tapin~ 200").length, 2);
      assert.equal(find(p, "tapout~ 30").length, 2);
      assert.equal(find(p, "+~ 30.").length, 2);
      assert.equal(find(p, "cos~").length, 2);
      one(p, "+~ 0.25");
      assert.equal(find(p, "delay~ 16384 8192").length, 2);
      one(p, "r ---vib_rate");
    });
  }

  for (const v of expect.versions.filter((x) => x.startsWith("doubles") && x !== "doubles-vibrato")) {
    test(`${name}: ${v} drifts only when the device has drift, each double on its own rand~`, () => {
      const p = sub(v).patcher;
      const rands = boxesOf(p).filter((b) => (b.text || "").startsWith("rand~"));
      assert.equal(rands.length, expect.drift ? 2 : 0);
      if (expect.drift && v === "doubles-pitchshift") assert.equal(find(p, "snapshot~ 20").length, 2);
    });
  }

  test(`${name}: every delay line shifter is two Hann-windowed taps swept by a phasor~`, () => {
    for (const { name: where, patcher: p } of allPatchers(patcher).filter((x) => x.name === "p dlshift")) {
      one(p, "tapin~ 200");
      one(p, "tapout~ 3 3");
      // Ramp frequency at control rate, smoothed into the phasor~.
      const rate = one(p, "expr (1. - pow(2.\\, $f1 / 1200.)) * 1000. / max($f2\\, 1.)");
      const [both] = linesTo(p, rate.id, 0).map((l) => box(p, l.source[0]));
      assert.equal(both.text, "pak 0. 30.");
      const [ramp] = targets(p, rate.id, 0);
      assert.equal(ramp.text, "$1 20");
      const [line] = targets(p, ramp.id, 0);
      assert.equal(line.text, "line~");
      assert.ok(targets(p, line.id, 0).some((b) => b.text === "phasor~"), where);
      one(p, "%~ 1.");
      assert.equal(find(p, "cos~").length, 2);
      assert.equal(find(p, "+~ 3.").length, 2);
    }
  });

  test(`${name}: panel has exactly the expected controls, with their ranges and defaults`, () => {
    const found = parameters(patcher);
    assert.deepEqual(found.map(longname).sort(), Object.keys(expect.controls).sort());
    for (const b of found) {
      const [maxclass, min, max, initial, items, unitstyle] = expect.controls[longname(b)];
      const v = b.saved_attribute_attributes.valueof;
      const n = longname(b);
      assert.equal(b.maxclass, maxclass, n);
      assert.equal(v.parameter_mmin, min, n);
      assert.equal(v.parameter_mmax, max, n);
      assert.deepEqual(v.parameter_initial, [initial], n);
      assert.equal(v.parameter_initial_enable, 1, n);
      if (items) assert.deepEqual(v.parameter_enum, items, n);
      if (unitstyle) assert.equal(v.parameter_unitstyle, unitstyle, n);
      assert.deepEqual(patcher.parameters[b.id], [n, v.parameter_shortname, 0]);
      const r = b.presentation_rect;
      assert.ok(b.presentation === 1 && r[0] >= 0 && r[0] + r[2] <= patcher.devicewidth && r[1] >= 0 && r[1] + r[3] <= 169, `${n} is off the panel`);
      assert.ok(targets(patcher, b.id, 0).some((t) => (t.text || "").startsWith("s ---")), `${n} sends nothing`);
    }
  });

  test(`${name}: no two panel items overlap`, () => {
    const shown = boxesOf(patcher).filter((b) => b.presentation);
    for (let i = 0; i < shown.length; i++) {
      for (let j = i + 1; j < shown.length; j++) {
        const [a, c] = [shown[i].presentation_rect, shown[j].presentation_rect];
        const overlap = a[0] < c[0] + c[2] && c[0] < a[0] + a[2] && a[1] < c[1] + c[3] && c[1] < a[1] + a[3];
        assert.ok(!overlap, `${shown[i].text || shown[i].varname} overlaps ${shown[j].text || shown[j].varname}`);
      }
    }
  });

  test(`${name}: readouts for each pitchshift~ layer, Defined Latency, and a warning line`, () => {
    const presented = boxesOf(patcher).filter((b) => b.presentation);
    assert.ok(presented.some((b) => b.text === "Defined Latency: 8192 smp"));
    for (const b of presented) {
      const r = b.presentation_rect;
      assert.ok(r[0] + r[2] <= patcher.devicewidth && r[1] + r[3] <= 169, `${b.text || b.varname} is off the panel`);
    }
    const expected = { "doubles-pitchshift": "d_readout", "interval-pitchshift": "i_readout" };
    for (const [v, varname] of Object.entries(expected)) {
      const c = boxesOf(patcher).find((b) => b.varname === varname);
      assert.equal(Boolean(c), pitchshifts.includes(v), varname);
      if (c) assert.ok(linesTo(patcher, c.id, 0).length > 0, `${varname} is never set`);
    }
    const warning = boxesOf(patcher).find((b) => b.varname === "warning");
    assert.equal(Boolean(warning), pitchshifts.length > 0);
  });
}
