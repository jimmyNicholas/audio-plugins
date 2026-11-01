// Generates "Slush Pitch Layer Audition.maxpat" from the description below, so the patcher
// JSON stays consistent and reviewable. Build spec: SPEC.md in this folder.
//
//   node build-patch.mjs            writes the .maxpat
//   npm run build                   writes the .maxpat and packs the .amxd (type audio)
//
// The patch is plain MSP and live.* objects: no gen~, no js, no packages. Every version of each
// layer is its own embedded subpatcher with a mute~ in front. Controls reach the subpatchers by
// device-local send and receive names (the "---" prefix, which Max for Live makes unique per
// device), so the wiring stays short.

import fs from "node:fs";
import { fileURLToPath } from "node:url";

export const L_MAX = 8192; // Defined Latency, samples. Never changes at runtime.
const DELAY_MAX = 16384; // delay~ buffer, samples
const DELAY_LMAX = `delay~ ${DELAY_MAX} ${L_MAX}`;
const LEVEL_MIN = -70; // the Level dials' minimum, treated as -inf (the layer is muted)
const MUTE_BELOW = -69.9;
// Control defaults that the logic also needs as starting values.
const DEFAULT = { doublesLevel: -9, intervalLevel: -12, cents: 12, driftDepth: 3 };
const DEVICE_WIDTH = 612;
const DEVICE_HEIGHT = 169;

const APPVERSION = { major: 8, minor: 5, revision: 4, architecture: "x64", modernui: 1 };

// ---- object shapes -----------------------------------------------------------------------
// Inlet and outlet counts for every object used, so lines can be checked by the tests.
// Max recomputes them on load; these only need to match what Max 8 builds.

function maxIndex(text) {
  let max = 0;
  for (const m of text.matchAll(/\$[ifsv](\d+)/g)) max = Math.max(max, Number(m[1]));
  return max;
}

function shape(text) {
  const [name, ...args] = text.split(" ");
  const sig = (n) => Array(n).fill("signal");
  const any = (n) => Array(n).fill("");
  switch (name) {
    case "plugin~": return [2, sig(2)];
    case "plugout~": return [2, []];
    case "live.thisdevice": return [1, ["bang", "int", "int"]];
    case "delay~": return [2, sig(1)];
    case "mute~": return [1, any(1)];
    case "*~": case "+~": case "%~": return [2, sig(1)];
    case "line~": return [2, ["signal", "bang"]];
    case "sig~": case "cos~": case "rand~": return [1, sig(1)];
    case "phasor~": return [2, sig(1)];
    case "tapin~": return [1, ["tapconnect"]];
    case "tapout~": return [args.length, sig(args.length)];
    case "expr~": return [maxIndex(text), sig(1)];
    case "expr": return [maxIndex(text), any(1)];
    case "selector~": return [Number(args[0]) + 1, sig(1)];
    case "snapshot~": return [2, ["float"]];
    case "pitchshift~": return [2, ["signal", ""]];
    case "dspstate~": return [1, ["int", "float", "int", "int"]];
    case "route": return [2, any(args.length + 1)];
    case "pak": case "pack": return [args.length, any(1)];
    case "change": return [1, ["", "int", "int"]];
    case "sel": return [args.length === 1 ? 2 : 1, [...Array(args.length).fill("bang"), ""]];
    case "t": return [1, args.map((a) => ({ b: "bang", i: "int", f: "float" })[a] || "")];
    case "delay": return [2, ["bang"]];
    case "s": case "print": return [1, []];
    case "r": return [0, any(1)];
    case "deferlow": case "loadmess": case "prepend": return [1, any(1)];
    case "sprintf": return [(text.match(/%[^%]/g) || []).length, any(1)];
    case "+": return [2, ["int"]];
    default: throw new Error("No shape for object: " + text);
  }
}

// ---- patch builder ------------------------------------------------------------------------

class Patch {
  constructor() {
    this.boxes = [];
    this.lines = [];
    this.count = 0;
    this.x = 20;
    this.y = 20;
  }

  // Patching position for the next boxes: a column at x, stepping down from y.
  at(x, y) {
    this.x = x;
    this.y = y;
    return this;
  }

  place(width, height = 22) {
    const rect = [this.x, this.y, width, height];
    this.y += height + 8;
    return rect;
  }

  add(box) {
    const id = `obj-${++this.count}`;
    this.boxes.push({ box: { id, ...box } });
    return id;
  }

  obj(text) {
    const [numinlets, outlettype] = shape(text);
    return this.add({
      maxclass: "newobj",
      numinlets,
      numoutlets: outlettype.length,
      outlettype,
      patching_rect: this.place(Math.max(40, text.length * 6 + 10)),
      text,
    });
  }

  msg(text) {
    return this.add({
      maxclass: "message",
      numinlets: 2,
      numoutlets: 1,
      outlettype: [""],
      patching_rect: this.place(Math.max(30, text.length * 6 + 10)),
      text,
    });
  }

  comment(text, extra = {}) {
    return this.add({
      maxclass: "comment",
      numinlets: 1,
      numoutlets: 0,
      patching_rect: this.place(Math.min(400, text.length * 6 + 10)),
      text,
      ...extra,
    });
  }

  inlet(index, comment, signal) {
    return this.add({
      maxclass: "inlet",
      comment,
      index,
      numinlets: 0,
      numoutlets: 1,
      outlettype: [signal ? "signal" : ""],
      patching_rect: [20 + (index - 1) * 180, 10, 30, 30],
    });
  }

  outlet(index, comment, y) {
    return this.add({
      maxclass: "outlet",
      comment,
      index,
      numinlets: 1,
      numoutlets: 0,
      patching_rect: [20 + (index - 1) * 180, y, 30, 30],
    });
  }

  // An embedded subpatcher box. build(inner) fills the inner patch and returns nothing.
  sub(name, build) {
    const inner = new Patch();
    build(inner);
    const inlets = inner.boxes.filter((b) => b.box.maxclass === "inlet").length;
    const outlets = inner.boxes.filter((b) => b.box.maxclass === "outlet").length;
    return this.add({
      maxclass: "newobj",
      numinlets: inlets,
      numoutlets: outlets,
      outlettype: Array(outlets).fill("signal"),
      patching_rect: this.place(Math.max(80, name.length * 6 + 20)),
      text: `p ${name}`,
      patcher: inner.toJSON([60, 120, 900, 700]),
    });
  }

  connect(from, outlet, to, inlet) {
    this.lines.push({ patchline: { destination: [to, inlet], source: [from, outlet] } });
  }

  toJSON(rect) {
    return {
      fileversion: 1,
      appversion: APPVERSION,
      classnamespace: "box",
      rect,
      gridsize: [8, 8],
      boxes: this.boxes,
      lines: this.lines,
    };
  }
}

// ---- the delay line shifter ---------------------------------------------------------------
// One pitch shifter made of a tapin~/tapout~ pair with two taps, each swept by a phasor~ and
// faded in and out by a Hann window. The two windows are half a cycle apart and sum to 1.
//   inlet 1: audio   inlet 2: shift in cents (signal)   inlet 3: window in ms (float)

function dlshift(p) {
  const audio = p.inlet(1, "audio", true);
  const cents = p.inlet(2, "shift in cents (signal)", true);
  const windowMs = p.inlet(3, "window in ms (float)", false);

  p.at(20, 60);
  p.comment("Ramp frequency f = (1 - 2^(cents/1200)) * 1000 / window. Negative for upward shifts, so the phasor~ runs backwards.");
  const rate = p.obj("expr~ (1. - pow(2.\\, $v1 / 1200.)) * 1000. / max($f2\\, 1.)");
  const phase1 = p.obj("phasor~");
  p.comment("Second tap: the same ramp, half a cycle on.");
  const half = p.obj("+~ 0.5");
  const phase2 = p.obj("%~ 1.");
  p.connect(cents, 0, rate, 0);
  p.connect(windowMs, 0, rate, 1);
  p.connect(rate, 0, phase1, 0);
  p.connect(phase1, 0, half, 0);
  p.connect(half, 0, phase2, 0);

  p.at(20, 300);
  p.comment("Tap delay (ms) = 3 + phase * window. 3 ms keeps it above one signal vector.");
  const time1 = p.obj("*~ 30.");
  const base1 = p.obj("+~ 3.");
  p.at(200, 330);
  const time2 = p.obj("*~ 30.");
  const base2 = p.obj("+~ 3.");
  p.connect(phase1, 0, time1, 0);
  p.connect(phase2, 0, time2, 0);
  p.connect(windowMs, 0, time1, 1);
  p.connect(windowMs, 0, time2, 1);
  p.connect(time1, 0, base1, 0);
  p.connect(time2, 0, base2, 0);

  p.at(380, 60);
  const tapin = p.obj("tapin~ 200");
  p.at(380, 420);
  const tapout = p.obj("tapout~ 3 3");
  p.connect(audio, 0, tapin, 0);
  p.connect(tapin, 0, tapout, 0);
  p.connect(base1, 0, tapout, 0);
  p.connect(base2, 0, tapout, 1);

  p.at(600, 300);
  p.comment("Hann window: 0.5 - 0.5 * cos~(phase). cos~ takes its input in cycles.");
  const windows = [phase1, phase2].map((phase) => {
    const cos = p.obj("cos~");
    const scale = p.obj("*~ -0.5");
    const offset = p.obj("+~ 0.5");
    p.connect(phase, 0, cos, 0);
    p.connect(cos, 0, scale, 0);
    p.connect(scale, 0, offset, 0);
    return offset;
  });

  p.at(380, 480);
  const out = p.outlet(1, "shifted audio", 560);
  for (const tap of [0, 1]) {
    const gain = p.obj("*~");
    p.connect(tapout, tap, gain, 0);
    p.connect(windows[tap], 0, gain, 1);
    p.connect(gain, 0, out, 0);
  }
}

// ---- shared pieces of the version subpatchers ---------------------------------------------

// The three inlets every version has: the mute~ control, then left and right audio.
function versionInlets(p) {
  p.inlet(1, "mute~ control", false);
  const left = p.inlet(2, "left in", true);
  const right = p.inlet(3, "right in", true);
  return { left, right };
}

// (L + R) * 0.5
function monoSum(p, left, right) {
  p.at(20, 60);
  const sum = p.obj("+~");
  const half = p.obj("*~ 0.5");
  p.connect(left, 0, sum, 0);
  p.connect(right, 0, sum, 1);
  p.connect(sum, 0, half, 0);
  return half;
}

// A selector~ 2 1 chosen by a 0 or 1 from a tab (via + 1). Returns the selector box.
function chooser(p, control, first, second) {
  const choose = p.obj("selector~ 2 1");
  p.connect(control, 0, choose, 0);
  p.connect(first, 0, choose, 1);
  p.connect(second, 0, choose, 2);
  return choose;
}

function tabIndexPlusOne(p, receiveName) {
  const r = p.obj(`r ${receiveName}`);
  const plus = p.obj("+ 1");
  p.connect(r, 0, plus, 0);
  return plus;
}

// A pitchshift~ followed by its compensation delay~ (Lmax - measured latency). The latency
// message goes through route latency to a trigger whose outlets are returned so callers can
// add the print and the readout. The compensation is always the trigger's rightmost outlet.
function compensatedPitchshift(p, input, triggerText) {
  const ps = p.obj("pitchshift~ @quality good @reportlatency 1");
  const delay = p.obj(DELAY_LMAX);
  const route = p.obj("route latency");
  const trigger = p.obj(triggerText);
  const compensation = p.obj(`expr max(${L_MAX} - $i1\\, 0)`);
  p.connect(input, 0, ps, 0);
  p.connect(ps, 0, delay, 0);
  p.connect(ps, 1, route, 0);
  p.connect(route, 0, trigger, 0);
  const last = triggerText.split(" ").length - 2;
  p.connect(trigger, last, compensation, 0);
  p.connect(compensation, 0, delay, 1);
  return { ps, delay, trigger };
}

// ---- the four versions --------------------------------------------------------------------

function doublesInputs(p) {
  const { left, right } = versionInlets(p);
  const mono = monoSum(p, left, right);
  p.comment("Source: Per channel (L up, R down) or Mono sum into both.");
  const source = tabIndexPlusOne(p, "---d_source");
  const upIn = chooser(p, source, left, mono);
  const downIn = chooser(p, source, right, mono);
  return { upIn, downIn };
}

function doublesDelayline(p) {
  const { upIn, downIn } = doublesInputs(p);

  p.at(260, 60);
  p.comment("Cents, smoothed. Drift: a rand~ per double times the depth (0 when Drift is off).");
  const cents = p.obj("r ---d_cents");
  const centsRamp = p.msg("$1 20");
  const centsSig = p.obj("line~");
  p.connect(cents, 0, centsRamp, 0);
  p.connect(centsRamp, 0, centsSig, 0);
  const amount = p.obj("r ---drift_amt");
  const amountRamp = p.msg("$1 50");
  const amountSig = p.obj("line~");
  p.connect(amount, 0, amountRamp, 0);
  p.connect(amountRamp, 0, amountSig, 0);
  const rate = p.obj("r ---drift_rate");

  const shifted = (sign) => {
    const random = p.obj("rand~ 0.2");
    const drift = p.obj("*~");
    const total = p.obj("+~");
    p.connect(rate, 0, random, 0);
    p.connect(random, 0, drift, 0);
    p.connect(amountSig, 0, drift, 1);
    p.connect(centsSig, 0, total, 0);
    p.connect(drift, 0, total, 1);
    if (sign > 0) return total;
    const negate = p.obj("*~ -1.");
    p.connect(total, 0, negate, 0);
    return negate;
  };

  p.at(520, 60);
  p.comment("Up double: +(cents + drift). Down double: -(cents + drift).");
  const upCents = shifted(1);
  p.at(700, 90);
  const downCents = shifted(-1);

  p.at(260, 420);
  p.comment("Doubles window: fixed 30 ms.");
  const windowMs = p.obj("loadmess 30.");
  const upShift = p.sub("dlshift", dlshift);
  p.at(520, 450);
  const downShift = p.sub("dlshift", dlshift);
  for (const [shift, input, shiftCents] of [[upShift, upIn, upCents], [downShift, downIn, downCents]]) {
    p.connect(input, 0, shift, 0);
    p.connect(shiftCents, 0, shift, 1);
    p.connect(windowMs, 0, shift, 2);
  }

  p.at(260, 520);
  p.comment("Delayed by Lmax to line up with the dry path. The shifter's own small delay is not compensated.");
  const upDelay = p.obj(DELAY_LMAX);
  p.at(520, 550);
  const downDelay = p.obj(DELAY_LMAX);
  p.connect(upShift, 0, upDelay, 0);
  p.connect(downShift, 0, downDelay, 0);
  p.connect(upDelay, 0, p.outlet(1, "left: up double", 640), 0);
  p.connect(downDelay, 0, p.outlet(2, "right: down double", 640), 0);
}

const QUALITIES = ["basic", "good", "better", "best"];

// The Quality menu, latency measuring, the Max Console line and the panel readout for a pair of
// compensated pitchshift~ objects (from compensatedPitchshift with "t i i i" for the first and
// "t i i" for the second). Quality sets both and measures now and again 250 ms later; so does
// the ---measure send (on load, and whenever a version is switched on).
function qualityAndLatency(p, x, layer, first, second, readoutName) {
  const both = [first.ps, second.ps];
  p.at(x, 60);
  p.comment("Quality: set both pitchshift~ objects, then measure now and again 250 ms later.");
  const quality = p.obj("r ---i_quality");
  const order = p.obj("t b i");
  const pick = p.obj("sel 0 1 2 3");
  p.connect(quality, 0, order, 0);
  p.connect(order, 1, pick, 0);
  const name = p.obj("pack 0 s");
  QUALITIES.forEach((q, i) => {
    p.at(x + i * 90, 210);
    const setQuality = p.msg(`quality ${q}`);
    const remember = p.msg(`symbol ${q}`);
    p.connect(pick, i, setQuality, 0);
    p.connect(pick, i, remember, 0);
    for (const ps of both) p.connect(setQuality, 0, ps, 0);
    p.connect(remember, 0, name, 1);
  });

  p.at(x + 360, 60);
  const measure = p.obj("r ---measure");
  const now = p.obj("t b b");
  const getlatency = p.msg("getlatency");
  const later = p.obj("delay 250");
  const getlatencyAgain = p.msg("getlatency");
  p.connect(order, 0, now, 0);
  p.connect(measure, 0, now, 0);
  p.connect(now, 1, getlatency, 0);
  p.connect(now, 0, later, 0);
  p.connect(later, 0, getlatencyAgain, 0);
  for (const ps of both) {
    p.connect(getlatency, 0, ps, 0);
    p.connect(getlatencyAgain, 0, ps, 0);
  }

  p.at(x + 180, 520);
  const line = p.msg(`pitchshift ${layer} $2 latency $1 samples`);
  const print = p.obj("print audition");
  const readout = p.obj(`s ${readoutName}`);
  p.connect(first.trigger, 1, name, 0);
  p.connect(second.trigger, 0, name, 0);
  p.connect(name, 0, line, 0);
  p.connect(line, 0, print, 0);
  p.connect(first.trigger, 0, readout, 0);
}

function doublesPitchshift(p) {
  const { upIn, downIn } = doublesInputs(p);

  p.at(260, 60);
  p.comment("Drift steps the pitch factor every 20 ms.");
  const cents = p.obj("r ---d_cents");
  const rate = p.obj("r ---drift_rate");
  const amount = p.obj("r ---drift_amt");

  const factor = (expr) => {
    const random = p.obj("rand~ 0.2");
    const drift = p.obj("*~ 0.");
    const snapshot = p.obj("snapshot~ 20");
    const changed = p.obj("change -999.");
    const both = p.obj(`pak ${DEFAULT.cents}. 0.`);
    const ratio = p.obj(expr);
    const set = p.msg("pitchshift $1");
    p.connect(rate, 0, random, 0);
    p.connect(random, 0, drift, 0);
    p.connect(amount, 0, drift, 1);
    p.connect(drift, 0, snapshot, 0);
    p.connect(snapshot, 0, changed, 0);
    p.connect(cents, 0, both, 0);
    p.connect(changed, 0, both, 1);
    p.connect(both, 0, ratio, 0);
    p.connect(ratio, 0, set, 0);
    return set;
  };
  p.at(260, 150);
  const upFactor = factor("expr pow(2.\\, ($f1 + $f2) / 1200.)");
  p.at(520, 150);
  const downFactor = factor("expr pow(2.\\, (0. - $f1 - $f2) / 1200.)");

  p.at(20, 420);
  const up = compensatedPitchshift(p, upIn, "t i i i");
  p.at(400, 420);
  const down = compensatedPitchshift(p, downIn, "t i i");
  p.connect(upFactor, 0, up.ps, 0);
  p.connect(downFactor, 0, down.ps, 0);

  qualityAndLatency(p, 800, "doubles", up, down, "---d_ps_lat");

  p.connect(up.delay, 0, p.outlet(1, "left: up double", 760), 0);
  p.connect(down.delay, 0, p.outlet(2, "right: down double", 760), 0);
}

function intervalInputs(p) {
  const { left, right } = versionInlets(p);
  const mono = monoSum(p, left, right);
  p.comment("Stereo: Mono centre (one shifter on the sum, out to both sides) or Per channel.");
  const stereo = tabIndexPlusOne(p, "---i_stereo");
  const leftIn = chooser(p, stereo, mono, left);
  return { leftIn, right, stereo };
}

// Left output is always the left shifter. Right is the left shifter (mono centre) or the right.
function intervalOutputs(p, stereo, leftOut, rightOut, y) {
  const rightChoice = chooser(p, stereo, leftOut, rightOut);
  p.connect(leftOut, 0, p.outlet(1, "left", y), 0);
  p.connect(rightChoice, 0, p.outlet(2, "right", y), 0);
}


function intervalPitchshift(p) {
  const { leftIn, right, stereo } = intervalInputs(p);

  p.at(260, 60);
  p.comment("Interval in cents (-1200 or -700) to a pitch factor.");
  const cents = p.obj("r ---i_cents");
  const ratio = p.obj("expr pow(2.\\, $f1 / 1200.)");
  const set = p.msg("pitchshift $1");
  p.connect(cents, 0, ratio, 0);
  p.connect(ratio, 0, set, 0);

  p.at(20, 420);
  const leftPs = compensatedPitchshift(p, leftIn, "t i i i");
  p.at(400, 420);
  const rightPs = compensatedPitchshift(p, right, "t i i");
  for (const ps of [leftPs.ps, rightPs.ps]) p.connect(set, 0, ps, 0);

  qualityAndLatency(p, 520, "interval", leftPs, rightPs, "---i_ps_lat");

  p.at(20, 700);
  intervalOutputs(p, stereo, leftPs.delay, rightPs.delay, 760);
}

function intervalDelayline(p) {
  const { leftIn, right, stereo } = intervalInputs(p);

  p.at(260, 60);
  p.comment("Interval in cents (-1200 or -700), and the Window dial.");
  const cents = p.obj("r ---i_cents");
  const centsSig = p.obj("sig~");
  const windowMs = p.obj("r ---i_window");
  p.connect(cents, 0, centsSig, 0);

  p.at(260, 300);
  const leftShift = p.sub("dlshift", dlshift);
  p.at(520, 300);
  const rightShift = p.sub("dlshift", dlshift);
  for (const [shift, input] of [[leftShift, leftIn], [rightShift, right]]) {
    p.connect(input, 0, shift, 0);
    p.connect(centsSig, 0, shift, 1);
    p.connect(windowMs, 0, shift, 2);
  }

  p.at(260, 380);
  p.comment("Delayed by Lmax to line up with the dry path. The shifter's own small delay is not compensated.");
  const leftDelay = p.obj(DELAY_LMAX);
  p.at(520, 410);
  const rightDelay = p.obj(DELAY_LMAX);
  p.connect(leftShift, 0, leftDelay, 0);
  p.connect(rightShift, 0, rightDelay, 0);

  p.at(260, 480);
  intervalOutputs(p, stereo, leftDelay, rightDelay, 600);
}

// ---- panel controls -------------------------------------------------------------------------

function parameterBox(p, maxclass, longname, shortname, rect, valueof, extra = {}) {
  const outlets = {
    "live.dial": ["", "float"],
    "live.text": ["", ""],
    "live.tab": ["", "", "float"],
    "live.menu": ["", "", "float"],
  }[maxclass];
  return p.add({
    maxclass,
    varname: longname.toLowerCase().replace(/ /g, "_"),
    numinlets: 1,
    numoutlets: outlets.length,
    outlettype: outlets,
    parameter_enable: 1,
    ...extra,
    patching_rect: p.place(rect[2], rect[3]),
    presentation: 1,
    presentation_rect: rect,
    saved_attribute_attributes: {
      valueof: {
        parameter_longname: longname,
        parameter_shortname: shortname,
        parameter_invisible: 1, // Stored Only: saved with the Set, not automatable
        parameter_initial_enable: 1,
        ...valueof,
      },
    },
  });
}

function dial(p, longname, shortname, x, y, { min, max, initial, unitstyle, type = 0 }) {
  const valueof = {
    parameter_type: type,
    parameter_mmin: min,
    parameter_mmax: max,
    parameter_initial: [initial],
    parameter_unitstyle: unitstyle,
  };
  if (type === 1) valueof.parameter_steps = max - min + 1;
  return parameterBox(p, "live.dial", longname, shortname, [x, y, 44, 48], valueof);
}

function choice(p, maxclass, longname, shortname, rect, items, initial, extra = {}) {
  return parameterBox(
    p,
    maxclass,
    longname,
    shortname,
    rect,
    {
      parameter_type: 2,
      parameter_mmin: 0,
      parameter_mmax: items.length - 1,
      parameter_initial: [initial],
      parameter_unitstyle: 9,
      parameter_enum: items,
    },
    extra,
  );
}

function tab(p, longname, shortname, rect, items, initial) {
  return choice(p, "live.tab", longname, shortname, rect, items, initial, { num_lines_patching: 1, num_lines_presentation: 1 });
}

function toggle(p, longname, rect, off, on, initial) {
  return choice(p, "live.text", longname, longname, rect, ["off", "on"], initial, { mode: 1, text: off, texton: on });
}

function label(p, text, rect) {
  return p.add({
    maxclass: "live.comment",
    numinlets: 1,
    numoutlets: 0,
    patching_rect: p.place(rect[2], rect[3]),
    presentation: 1,
    presentation_rect: rect,
    text,
  });
}

function readout(p, varname, text, rect) {
  return p.add({
    maxclass: "comment",
    varname,
    linecount: Math.round(rect[3] / 13),
    numinlets: 1,
    numoutlets: 0,
    patching_rect: p.place(rect[2], rect[3]),
    presentation: 1,
    presentation_rect: rect,
    text,
  });
}

const UNIT_DB = 4; // parameter_unitstyle for dB

// Builds every control, each sending its value to a device-local name. Returns the boxes
// so the load bang can make each one output its value.
function panel(p) {
  p.at(20, 520);
  const controls = [];
  const send = (box, name) => {
    controls.push(box);
    const s = p.obj(`s ---${name}`);
    p.connect(box, 0, s, 0);
  };

  // Layers column
  label(p, "LAYERS", [4, 2, 110, 14]);
  send(toggle(p, "Layers", [4, 18, 70, 15], "Layers Off", "Layers On", 1), "layers");
  label(p, "Solo", [4, 36, 40, 14]);
  send(tab(p, "Solo", "Solo", [4, 50, 112, 15], ["Off", "Doubles", "Interval"], 0), "solo");
  readout(p, "defined_latency", `Defined Latency: ${L_MAX} smp`, [4, 72, 116, 13]);

  // Doubles column
  p.at(260, 520);
  const dx = 128;
  label(p, "DOUBLES", [dx, 2, 120, 14]);
  label(p, "Engine", [dx, 18, 40, 14]);
  send(tab(p, "D Engine", "Engine", [dx + 42, 18, 120, 15], ["Delay line", "Pitchshift"], 0), "d_engine");
  label(p, "Source", [dx, 36, 40, 14]);
  send(tab(p, "D Source", "Source", [dx + 42, 36, 120, 15], ["Per channel", "Mono sum"], 0), "d_source");
  send(toggle(p, "D Drift", [dx + 184, 36, 90, 15], "Drift Off", "Drift On", 0), "drift_on");
  const dials = [
    ["D Cents", "Cents", { min: 0, max: 30, initial: DEFAULT.cents, unitstyle: 0, type: 1 }, "d_cents"],
    ["D Level", "Level", { min: LEVEL_MIN, max: 0, initial: DEFAULT.doublesLevel, unitstyle: UNIT_DB }, "d_level"],
    ["D Trim DL", "Trim DL", { min: -12, max: 12, initial: 0, unitstyle: UNIT_DB }, "d_trim_dl"],
    ["D Trim PS", "Trim PS", { min: -12, max: 12, initial: 0, unitstyle: UNIT_DB }, "d_trim_ps"],
    ["D Drift Depth", "Depth", { min: 0, max: 10, initial: DEFAULT.driftDepth, unitstyle: 1 }, "drift_depth"],
    ["D Drift Rate", "Rate", { min: 0.05, max: 1, initial: 0.2, unitstyle: 3 }, "drift_rate"],
  ];
  dials.forEach(([longname, shortname, range, name], i) => {
    send(dial(p, longname, shortname, dx + i * 46, 58, range), name);
  });

  // Interval column
  p.at(500, 520);
  const ix = 418;
  label(p, "INTERVAL", [ix, 2, 120, 14]);
  label(p, "Engine", [ix, 18, 44, 14]);
  send(tab(p, "I Engine", "Engine", [ix + 46, 18, 120, 15], ["Pitchshift", "Delay line"], 0), "i_engine");
  label(p, "Quality", [ix, 36, 44, 14]);
  send(choice(p, "live.menu", "I Quality", "Quality", [ix + 46, 36, 80, 15], QUALITIES, 1), "i_quality");
  const intervalDials = [
    ["I Window", "Window", { min: 20, max: 150, initial: 60, unitstyle: 2 }, "i_window"],
    ["I Level", "Level", { min: LEVEL_MIN, max: 0, initial: DEFAULT.intervalLevel, unitstyle: UNIT_DB }, "i_level"],
    ["I Trim PS", "Trim PS", { min: -12, max: 12, initial: 0, unitstyle: UNIT_DB }, "i_trim_ps"],
    ["I Trim DL", "Trim DL", { min: -12, max: 12, initial: 0, unitstyle: UNIT_DB }, "i_trim_dl"],
  ];
  intervalDials.forEach(([longname, shortname, range, name], i) => {
    send(dial(p, longname, shortname, ix + i * 46, 58, range), name);
  });
  label(p, "Interval", [ix, 112, 44, 14]);
  send(tab(p, "I Interval", "Interval", [ix + 46, 112, 140, 15], ["Octave down", "Fifth down"], 0), "i_interval");
  label(p, "Stereo", [ix, 130, 44, 14]);
  send(tab(p, "I Stereo", "Stereo", [ix + 46, 130, 140, 15], ["Mono centre", "Per channel"], 0), "i_stereo");

  return controls;
}

// ---- readouts -------------------------------------------------------------------------------

function readouts(p) {
  p.at(760, 20);
  p.comment("Readouts: measured pitchshift~ latency in samples and ms, and a warning over Lmax.");
  const sr = p.obj("r ---sr");
  const show = (name, labelText, varname, rect) => {
    const r = p.obj(`r ---${name}`);
    const both = p.obj("t i i");
    const ms = p.obj("expr $f1 * 1000. / max($f2\\, 1.)");
    const format = p.obj(`sprintf set ${labelText}: %ld smp (%.1f ms)`);
    const text = readout(p, varname, `${labelText}: not measured yet`, rect);
    p.connect(r, 0, both, 0);
    p.connect(both, 1, ms, 0);
    p.connect(sr, 0, ms, 1);
    p.connect(ms, 0, format, 1);
    p.connect(both, 0, format, 0);
    p.connect(format, 0, text, 0);
  };
  show("d_ps_lat", "Doubles PS latency", "d_readout", [4, 88, 116, 26]);
  p.at(1000, 20);
  show("i_ps_lat", "Interval PS latency", "i_readout", [4, 114, 116, 26]);

  p.at(1240, 20);
  const doubles = p.obj("r ---d_ps_lat");
  const interval = p.obj("r ---i_ps_lat");
  const both = p.obj("pak 0 0");
  const which = p.obj(`expr ($i1 > ${L_MAX}) + 2 * ($i2 > ${L_MAX})`);
  const pick = p.obj("sel 0 1 2 3");
  const warning = readout(p, "warning", "", [4, 142, 116, 26]);
  const print = p.obj("print audition");
  p.connect(doubles, 0, both, 0);
  p.connect(interval, 0, both, 1);
  p.connect(both, 0, which, 0);
  p.connect(which, 0, pick, 0);
  const texts = [
    ["set", null],
    [`set Warning: doubles PS latency over ${L_MAX}`, `warning doubles pitchshift latency over ${L_MAX} samples`],
    [`set Warning: interval PS latency over ${L_MAX}`, `warning interval pitchshift latency over ${L_MAX} samples`],
    [`set Warning: both PS latencies over ${L_MAX}`, `warning both pitchshift latencies over ${L_MAX} samples`],
  ];
  texts.forEach(([panelText, consoleText], i) => {
    const m = p.msg(panelText);
    p.connect(pick, i, m, 0);
    p.connect(m, 0, warning, 0);
    if (consoleText) {
      const c = p.msg(consoleText);
      p.connect(pick, i, c, 0);
      p.connect(c, 0, print, 0);
    }
  });
}

// ---- switching logic ------------------------------------------------------------------------
// Each version is on when the layers are on, Solo allows its layer, its layer's Level is above
// the minimum, and its engine is selected. Turning on: unmute now, fade in 250 ms later (so
// stale audio in its delay buffers has flushed). Turning off: fade out now, mute 30 ms later.

function versionGain(p, x, { onExpr, layerPak, level, levelInitial, trim, mute, outputs }) {
  p.at(x, 760);
  const on = p.obj(onExpr);
  const changed = p.obj("change -1");
  const pick = p.obj("sel 1 0");
  p.connect(layerPak, 0, on, 0);
  p.connect(on, 0, changed, 0);
  p.connect(changed, 0, pick, 0);

  const gainPak = p.obj(`pak 0 ${levelInitial}. 0.`);
  const toTurnOn = p.obj("t b b b");
  const toTurnOff = p.obj("t b b b");
  const fadeInLater = p.obj("delay 250");
  const muteLater = p.obj("delay 30");
  const unmute = p.msg("0");
  const muteNow = p.msg("1");
  const stopMute = p.msg("stop");
  const stopFade = p.msg("stop");
  const gainOn = p.msg("1");
  const gainOff = p.msg("0");
  p.connect(pick, 0, toTurnOn, 0);
  p.connect(pick, 1, toTurnOff, 0);
  p.connect(toTurnOn, 2, unmute, 0);
  p.connect(toTurnOn, 1, stopMute, 0);
  p.connect(toTurnOn, 0, fadeInLater, 0);
  p.connect(stopMute, 0, muteLater, 0);
  p.connect(unmute, 0, mute, 0);
  p.connect(fadeInLater, 0, gainOn, 0);
  p.connect(fadeInLater, 0, p.obj("s ---measure"), 0); // a pitchshift~ muted at load may not have reported yet
  p.connect(gainOn, 0, gainPak, 0);
  p.connect(toTurnOff, 2, stopFade, 0);
  p.connect(toTurnOff, 1, gainOff, 0);
  p.connect(toTurnOff, 0, muteLater, 0);
  p.connect(stopFade, 0, fadeInLater, 0);
  p.connect(gainOff, 0, gainPak, 0);
  p.connect(muteLater, 0, muteNow, 0);
  p.connect(muteNow, 0, mute, 0);

  const levelR = p.obj(`r ---${level}`);
  const trimR = p.obj(`r ---${trim}`);
  const gain = p.obj("expr $i1 * pow(10.\\, ($f2 + $f3) / 20.)");
  const ramp = p.msg("$1 20");
  const line = p.obj("line~");
  p.connect(levelR, 0, gainPak, 1);
  p.connect(trimR, 0, gainPak, 2);
  p.connect(gainPak, 0, gain, 0);
  p.connect(gain, 0, ramp, 0);
  p.connect(ramp, 0, line, 0);
  for (const out of outputs) p.connect(line, 0, out, 1);
}

// ---- the device ----------------------------------------------------------------------------

export function buildPatcher() {
  const p = new Patch();

  p.at(20, 20);
  p.comment("Slush Pitch Layer Audition: a throwaway A/B device. Build spec: SPEC.md. Generated by build-patch.mjs; edit that, not this.");
  const plugin = p.obj("plugin~");
  p.at(20, 400);
  const plugout = p.obj("plugout~");

  // Dry path: delayed by exactly Lmax, at unity.
  p.at(20, 100);
  p.comment(`Dry: delayed by Lmax = ${L_MAX} samples, the Defined Latency.`);
  const dryGains = [0, 1].map((channel) => {
    p.at(20 + channel * 120, 130);
    const delay = p.obj(DELAY_LMAX);
    const gain = p.obj("*~ 0.");
    p.connect(plugin, channel, delay, 0);
    p.connect(delay, 0, gain, 0);
    p.connect(gain, 0, plugout, channel);
    return gain;
  });

  // The four versions, each behind a mute~, each with a gain stage per channel.
  const versions = [
    ["doubles-delayline", doublesDelayline],
    ["doubles-pitchshift", doublesPitchshift],
    ["interval-pitchshift", intervalPitchshift],
    ["interval-delayline", intervalDelayline],
  ].map(([name, build], i) => {
    const x = 280 + i * 240;
    p.at(x, 130);
    const mute = p.obj("mute~");
    const version = p.sub(name, build);
    p.connect(mute, 0, version, 0);
    p.connect(plugin, 0, version, 1);
    p.connect(plugin, 1, version, 2);
    const outputs = [0, 1].map((channel) => {
      p.at(x + channel * 80, 220);
      const gain = p.obj("*~ 0.");
      p.connect(version, channel, gain, 0);
      p.connect(gain, 0, plugout, channel);
      return gain;
    });
    return { mute, outputs };
  });

  const controls = panel(p);
  readouts(p);

  // Load: make every control output its value, read the sample rate, then measure latency.
  p.at(20, 460);
  const device = p.obj("live.thisdevice");
  const load = p.obj("t b b b");
  const dsp = p.obj("dspstate~");
  const srSend = p.obj("s ---sr");
  const defer = p.obj("deferlow");
  const measure = p.obj("s ---measure");
  const again = p.obj("delay 1000");
  const measureAgain = p.obj("s ---measure");
  p.connect(device, 0, load, 0);
  for (const c of controls) p.connect(load, 2, c, 0);
  p.connect(load, 1, dsp, 0);
  p.connect(dsp, 1, srSend, 0);
  p.connect(load, 0, defer, 0);
  p.connect(defer, 0, measure, 0);
  p.connect(defer, 0, again, 0);
  p.connect(again, 0, measureAgain, 0);

  // Dry: on unless Solo is on (Layers Off always gives the plain delayed input).
  p.at(20, 760);
  p.comment("Switching");
  const layers = p.obj("r ---layers");
  const solo = p.obj("r ---solo");
  const dryPak = p.obj("pak 1 0");
  const dryOn = p.obj("expr $i1 == 0 || $i2 == 0");
  const dryRamp = p.msg("$1 20");
  const dryLine = p.obj("line~");
  p.connect(layers, 0, dryPak, 0);
  p.connect(solo, 0, dryPak, 1);
  p.connect(dryPak, 0, dryOn, 0);
  p.connect(dryOn, 0, dryRamp, 0);
  p.connect(dryRamp, 0, dryLine, 0);
  for (const g of dryGains) p.connect(dryLine, 0, g, 1);

  // Per layer: layers, solo, level, engine.
  const layerPak = (x, levelName, levelInitial, engineName) => {
    p.at(x, 700);
    const pak = p.obj(`pak 1 0 ${levelInitial}. 0`);
    const level = p.obj(`r ---${levelName}`);
    const engine = p.obj(`r ---${engineName}`);
    p.connect(layers, 0, pak, 0);
    p.connect(solo, 0, pak, 1);
    p.connect(level, 0, pak, 2);
    p.connect(engine, 0, pak, 3);
    return pak;
  };
  const doubles = { layerPak: layerPak(200, "d_level", DEFAULT.doublesLevel, "d_engine"), level: "d_level", levelInitial: DEFAULT.doublesLevel };
  const interval = { layerPak: layerPak(680, "i_level", DEFAULT.intervalLevel, "i_engine"), level: "i_level", levelInitial: DEFAULT.intervalLevel };

  const doublesOn = (engine) => `expr $i1 && $i2 != 2 && $f3 > ${MUTE_BELOW} && $i4 == ${engine}`;
  const intervalOn = (engine) => `expr $i1 && $i2 != 1 && $f3 > ${MUTE_BELOW} && $i4 == ${engine}`;
  const [doublesDelay, doublesPs, intervalPs, intervalDelay] = versions;
  versionGain(p, 200, { onExpr: doublesOn(0), ...doubles, trim: "d_trim_dl", ...doublesDelay });
  versionGain(p, 440, { onExpr: doublesOn(1), ...doubles, trim: "d_trim_ps", ...doublesPs });
  versionGain(p, 680, { onExpr: intervalOn(0), ...interval, trim: "i_trim_ps", ...intervalPs });
  versionGain(p, 920, { onExpr: intervalOn(1), ...interval, trim: "i_trim_dl", ...intervalDelay });

  // Derived values for the subpatchers.
  p.at(1240, 400);
  const driftOn = p.obj("r ---drift_on");
  const driftDepth = p.obj("r ---drift_depth");
  const driftPak = p.obj(`pak 0 ${DEFAULT.driftDepth}.`);
  const driftAmount = p.obj("expr $i1 * $f2");
  const driftSend = p.obj("s ---drift_amt");
  p.connect(driftOn, 0, driftPak, 0);
  p.connect(driftDepth, 0, driftPak, 1);
  p.connect(driftPak, 0, driftAmount, 0);
  p.connect(driftAmount, 0, driftSend, 0);
  const intervalTab = p.obj("r ---i_interval");
  const intervalCents = p.obj("expr -1200 + 500 * $i1");
  const centsSend = p.obj("s ---i_cents");
  p.connect(intervalTab, 0, intervalCents, 0);
  p.connect(intervalCents, 0, centsSend, 0);

  const parameters = { inherited_shortname: 1 };
  for (const { box } of p.boxes) {
    if (!box.saved_attribute_attributes) continue;
    const v = box.saved_attribute_attributes.valueof;
    parameters[box.id] = [v.parameter_longname, v.parameter_shortname, 0];
  }

  return {
    patcher: {
      fileversion: 1,
      appversion: APPVERSION,
      classnamespace: "box",
      rect: [100, 100, 1500, 1000],
      openrect: [0, 0, DEVICE_WIDTH, DEVICE_HEIGHT],
      devicewidth: DEVICE_WIDTH,
      openinpresentation: 1,
      default_fontsize: 10,
      default_fontname: "Arial Bold",
      gridsize: [8, 8],
      boxanimatetime: 500,
      latency: L_MAX,
      is_mpe: 0,
      minimum_live_version: "",
      minimum_max_version: "",
      platform_compatibility: 0,
      project: {
        version: 1,
        creationdate: 3590052786,
        modificationdate: 3590052786,
        viewrect: [25, 106, 300, 500],
        autoorganize: 1,
        hideprojectwindow: 1,
        showdependencies: 1,
        autolocalize: 0,
        contents: { patchers: {} },
        layout: {},
        searchpath: {},
        detailsvisible: 0,
        amxdtype: 1633771873,
        readonly: 0,
        devpathtype: 0,
        devpath: ".",
        sortmode: 0,
        viewmode: 0,
        includepackages: 0,
      },
      autosave: 0,
      parameters,
      boxes: p.boxes,
      lines: p.lines,
    },
  };
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const out = new URL("./Slush Pitch Layer Audition.maxpat", import.meta.url);
  fs.writeFileSync(out, JSON.stringify(buildPatcher(), null, 2) + "\n");
}
