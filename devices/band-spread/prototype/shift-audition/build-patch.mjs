// PROTOTYPE, throwaway. Generates the Band Spread shift audition for ticket #24 ("Does Band
// Spread shift by pitch or by frequency?"). One Max for Live audio effect: it cuts one Band
// out of the input and plays three Copies of it, through either a pitch shift engine or a
// frequency shift engine, so the two can be compared by ear in Live 11.
//
//   node build-patch.mjs   (or npm run build)   writes the .maxpat and packs the .amxd
//
// Plain MSP and live.* objects only: no gen~, no js, no packages. Each engine is its own
// embedded subpatcher behind a mute~, so only one runs at a time. Controls reach the
// subpatchers by device-local send and receive names (the "---" prefix). The patch builder
// and panel helpers are copied from the Slush Pitch Layer audition on purpose: this is
// throwaway code and should not depend on another device's prototype.

import fs from "node:fs";
import { fileURLToPath } from "node:url";
import { pack, AUDIO_EFFECT } from "../../../../tools/amxd.mjs";

export const FILE = "Band Spread Shift Audition";
export const L_MAX = 8192; // Defined Latency, samples. The audition proved 8192 lines up in Live.
const DELAY_LMAX = `delay~ ${L_MAX * 2} ${L_MAX}`;
export const COPIES = [
  { semis: -12, hz: -300, level: -6 },
  { semis: 7, hz: 400, level: -6 },
  { semis: 12, hz: 1000, level: -6 },
];
export const ENGINES = ["Pitch", "Freq match", "Freq Hz"];
const DEFAULT = { centre: 800, width: 0.5, stages: 1, gain: 12 };
const LEVEL_MIN = -70; // the Level dials' minimum, treated as off
const MUTE_BELOW = -69.9;
const DEVICE_HEIGHT = 169;

const APPVERSION = { major: 8, minor: 5, revision: 4, architecture: "x64", modernui: 1 };

// ---- object shapes -----------------------------------------------------------------------
// Inlet and outlet counts for every object used, so the tests can check every line.

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
    case "*~": case "+~": return [2, sig(1)];
    case "line~": return [2, ["signal", "bang"]];
    case "clip~": return [3, sig(1)];
    case "filtercoeff~": return [3, sig(5)];
    case "biquad~": return [6, sig(1)];
    case "freqshift~": return [2, sig(2)];
    case "pitchshift~": return [2, ["signal", ""]];
    case "selector~": return [Number(args[0]) + 1, sig(1)];
    case "expr": return [maxIndex(text), any(1)];
    case "route": return [2, any(args.length + 1)];
    case "pak": return [args.length, any(1)];
    case "change": return [1, ["", "int", "int"]];
    case "sel": return [args.length === 1 ? 2 : 1, [...Array(args.length).fill("bang"), ""]];
    case "t": return [1, args.map((a) => ({ b: "bang", i: "int", f: "float" })[a] || "")];
    case "delay": return [2, ["bang"]];
    case "s": return [1, []];
    case "r": return [0, any(1)];
    case "deferlow": case "loadmess": return [1, any(1)];
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

  comment(text) {
    return this.add({
      maxclass: "comment",
      numinlets: 1,
      numoutlets: 0,
      patching_rect: this.place(Math.min(400, text.length * 6 + 10)),
      text,
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

// A float that smooths to its new value over 20 ms: "$1 20" into line~. Returns the line~.
function smoothed(p, source, outlet = 0) {
  const ramp = p.msg("$1 20");
  const line = p.obj("line~");
  p.connect(source, outlet, ramp, 0);
  p.connect(ramp, 0, line, 0);
  return line;
}

// ---- the engines ----------------------------------------------------------------------------
// Both take inlet 1 (the mute~ control) and inlet 2 (the Band), and give one outlet per Copy,
// each already delayed so its total delay is Lmax.

function engineInlets(p) {
  p.inlet(1, "mute~ control", false);
  return p.inlet(2, "band", true);
}

// Pitch: one pitchshift~ per Copy at 2^(semitones / 12), then delay~ by Lmax minus its measured
// latency. Copy 1's latency also goes to the panel readout.
function pitchEngine(p) {
  const band = engineInlets(p);
  p.at(700, 60);
  p.comment("getlatency on load, and whenever this engine is switched on.");
  const measure = p.obj("r ---measure");
  const getlatency = p.msg("getlatency");
  p.connect(measure, 0, getlatency, 0);

  COPIES.forEach((_, k) => {
    const n = k + 1;
    p.at(20 + k * 220, 60);
    p.comment(`Copy ${n}: semitones to a pitch factor.`);
    const semis = p.obj(`r ---c${n}_semis`);
    const factor = p.obj("expr pow(2.\\, $f1 / 12.)");
    const set = p.msg("pitchshift $1");
    p.connect(semis, 0, factor, 0);
    p.connect(factor, 0, set, 0);

    const ps = p.obj("pitchshift~ @quality good @reportlatency 1");
    const delay = p.obj(DELAY_LMAX);
    const route = p.obj("route latency");
    const compensation = p.obj(`expr max(${L_MAX} - $i1\\, 0)`);
    p.connect(band, 0, ps, 0);
    p.connect(set, 0, ps, 0);
    p.connect(getlatency, 0, ps, 0);
    p.connect(ps, 0, delay, 0);
    p.connect(ps, 1, route, 0);
    if (k === 0) {
      const both = p.obj("t i i");
      const readout = p.obj("s ---ps_lat");
      p.connect(route, 0, both, 0);
      p.connect(both, 1, compensation, 0);
      p.connect(both, 0, readout, 0);
    } else {
      p.connect(route, 0, compensation, 0);
    }
    p.connect(compensation, 0, delay, 1);
    p.connect(delay, 0, p.outlet(n, `copy ${n}`, 600), 0);
  });
}

// Frequency: one freqshift~ per Copy, shifted by ---c<n>_fhz Hz (signed; the left outlet is the
// positive sideband, so a negative amount moves the Copy down), then delayed by Lmax. freqshift~
// has no block latency, so the fixed Lmax keeps it in time with the pitch engine.
function freqEngine(p) {
  const band = engineInlets(p);
  COPIES.forEach((_, k) => {
    const n = k + 1;
    p.at(20 + k * 220, 60);
    p.comment(`Copy ${n}: shift in Hz, smoothed over 20 ms.`);
    const hz = p.obj(`r ---c${n}_fhz`);
    const hzSig = smoothed(p, hz);
    const shifter = p.obj("freqshift~");
    const delay = p.obj(DELAY_LMAX);
    p.connect(band, 0, shifter, 0);
    p.connect(hzSig, 0, shifter, 1);
    p.connect(shifter, 0, delay, 0);
    p.connect(delay, 0, p.outlet(n, `copy ${n}`, 600), 0);
  });
}

// ---- panel ----------------------------------------------------------------------------------

const UNIT = { float: 1, hz: 3, db: 4, semitones: 7, enum: 9 };

function parameterBox(p, maxclass, longname, shortname, rect, valueof, extra = {}) {
  const outlets = {
    "live.dial": ["", "float"],
    "live.text": ["", ""],
    "live.tab": ["", "", "float"],
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
    fontname: "Arial",
    fontsize: 9,
    linecount: 1,
    numinlets: 1,
    numoutlets: 0,
    patching_rect: p.place(rect[2], rect[3]),
    presentation: 1,
    presentation_rect: rect,
    text,
  });
}

// Every control: longname, shortname, send name, kind and range, and where it sits.
const DIAL = [44, 48];
export function controls() {
  const list = [
    { longname: "Centre", shortname: "Centre", send: "centre", kind: "dial", min: 40, max: 12000, initial: DEFAULT.centre, unitstyle: UNIT.hz, exponent: 3, rect: [4, 16, ...DIAL] },
    { longname: "Width", shortname: "Width", send: "width", kind: "dial", min: 0.1, max: 3, initial: DEFAULT.width, unitstyle: UNIT.float, rect: [52, 16, ...DIAL] },
    { longname: "Stages", shortname: "Stages", send: "stages", kind: "tab", items: ["1", "2", "3"], initial: DEFAULT.stages, rect: [44, 70, 60, 15] },
    { longname: "Hear Band", shortname: "Band", send: "band", kind: "toggle", off: "Band Off", on: "Band On", initial: 0, rect: [4, 90, 100, 15] },
    { longname: "Engine", shortname: "Engine", send: "engine", kind: "tab", items: ENGINES, initial: 0, rect: [114, 18, 180, 15] },
    { longname: "Gain", shortname: "Gain", send: "gain", kind: "dial", min: -12, max: 36, initial: DEFAULT.gain, unitstyle: UNIT.db, rect: [114, 38, ...DIAL] },
  ];
  COPIES.forEach((c, k) => {
    const n = k + 1;
    const x = 304 + k * 50;
    list.push(
      { longname: `C${n} Semis`, shortname: "Semis", send: `c${n}_semis`, kind: "dial", min: -24, max: 24, initial: c.semis, unitstyle: UNIT.semitones, type: 1, rect: [x, 16, ...DIAL] },
      { longname: `C${n} Hz`, shortname: "Hz", send: `c${n}_hz`, kind: "dial", min: -3000, max: 3000, initial: c.hz, unitstyle: UNIT.hz, rect: [x, 66, ...DIAL] },
      { longname: `C${n} Level`, shortname: "Level", send: `c${n}_level`, kind: "dial", min: LEVEL_MIN, max: 0, initial: c.level, unitstyle: UNIT.db, rect: [x, 116, ...DIAL] },
    );
  });
  return list;
}

function controlBox(p, c) {
  if (c.kind === "dial") {
    const valueof = {
      parameter_type: c.type || 0,
      parameter_mmin: c.min,
      parameter_mmax: c.max,
      parameter_initial: [c.initial],
      parameter_unitstyle: c.unitstyle,
    };
    if (c.type === 1) valueof.parameter_steps = c.max - c.min + 1;
    if (c.exponent) valueof.parameter_exponent = c.exponent;
    return parameterBox(p, "live.dial", c.longname, c.shortname, c.rect, valueof);
  }
  const items = c.kind === "toggle" ? ["off", "on"] : c.items;
  const extra = c.kind === "toggle" ? { mode: 1, text: c.off, texton: c.on } : { num_lines_patching: 1, num_lines_presentation: 1 };
  return parameterBox(
    p,
    c.kind === "toggle" ? "live.text" : "live.tab",
    c.longname,
    c.shortname,
    c.rect,
    {
      parameter_type: 2,
      parameter_mmin: 0,
      parameter_mmax: items.length - 1,
      parameter_initial: [c.initial],
      parameter_unitstyle: UNIT.enum,
      parameter_enum: items,
    },
    extra,
  );
}

export const DEVICE_WIDTH = 304 + COPIES.length * 50 + 4;

function panel(p) {
  p.at(20, 1100);
  label(p, "BAND", [4, 2, 100, 14]);
  label(p, "Stages", [4, 70, 38, 14]);
  label(p, "ENGINE", [114, 2, 100, 14]);
  COPIES.forEach((_, k) => label(p, `COPY ${k + 1}`, [304 + k * 50, 2, 48, 14]));
  readout(p, "defined_latency", `Defined Latency: ${L_MAX} smp`, [162, 40, 132, 13]);
  const boxes = controls().map((c) => {
    const b = controlBox(p, c);
    p.connect(b, 0, p.obj(`s ---${c.send}`), 0);
    return b;
  });
  return boxes;
}

// ---- switching --------------------------------------------------------------------------------
// An engine turning on: unmute now, fade in 250 ms later (so stale audio in its buffers has
// gone) and, for the pitch engine, measure latency again. Turning off: fade out now, mute 30 ms
// later.

function engineSwitch(p, x, { engine, onExpr, mute, outputs, remeasure }) {
  p.at(x, 760);
  const on = p.obj(onExpr);
  const changed = p.obj("change -1");
  const pick = p.obj("sel 1 0");
  p.connect(engine, 0, on, 0);
  p.connect(on, 0, changed, 0);
  p.connect(changed, 0, pick, 0);

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
  if (remeasure) p.connect(fadeInLater, 0, p.obj("s ---measure"), 0);
  p.connect(toTurnOff, 2, stopFade, 0);
  p.connect(toTurnOff, 1, gainOff, 0);
  p.connect(toTurnOff, 0, muteLater, 0);
  p.connect(stopFade, 0, fadeInLater, 0);
  p.connect(muteLater, 0, muteNow, 0);
  p.connect(muteNow, 0, mute, 0);

  const ramp = p.msg("$1 20");
  const line = p.obj("line~");
  p.connect(gainOn, 0, ramp, 0);
  p.connect(gainOff, 0, ramp, 0);
  p.connect(ramp, 0, line, 0);
  for (const g of outputs) p.connect(line, 0, g, 1);
}

// ---- the device ----------------------------------------------------------------------------

export function buildPatcher() {
  const p = new Patch();

  p.at(20, 20);
  p.comment(`${FILE}: PROTOTYPE for Band Spread ticket #24. Generated by build-patch.mjs; edit that, not this.`);
  const plugin = p.obj("plugin~");

  // The Band: mono sum, then filtercoeff~ bandpass into up to three biquad~ in series.
  p.at(20, 100);
  p.comment("Band: (L + R) / 2 through 1 to 3 band pass stages from one filtercoeff~.");
  const sum = p.obj("+~");
  const half = p.obj("*~ 0.5");
  p.connect(plugin, 0, sum, 0);
  p.connect(plugin, 1, sum, 1);
  p.connect(sum, 0, half, 0);

  p.at(260, 100);
  const centre = p.obj("r ---centre");
  const centreSig = smoothed(p, centre);
  p.comment("Width in octaves N to Q = sqrt(2^N) / (2^N - 1).");
  const width = p.obj("r ---width");
  const q = p.obj("expr sqrt(pow(2.\\, $f1)) / (pow(2.\\, max($f1\\, 0.01)) - 1.)");
  p.connect(width, 0, q, 0);
  const qSig = smoothed(p, q);
  const unity = p.obj("loadmess 1.");
  const coeff = p.obj("filtercoeff~ bandpass");
  p.connect(centreSig, 0, coeff, 0);
  p.connect(unity, 0, coeff, 1);
  p.connect(qSig, 0, coeff, 2);

  p.at(20, 300);
  const stages = [0, 1, 2].map(() => p.obj("biquad~"));
  stages.forEach((stage, i) => {
    p.connect(i === 0 ? half : stages[i - 1], 0, stage, 0);
    for (let c = 0; c < 5; c++) p.connect(coeff, c, stage, c + 1);
  });
  const stagesTab = p.obj("r ---stages");
  const stagesIndex = p.obj("+ 1");
  const choose = p.obj("selector~ 3 1");
  p.connect(stagesTab, 0, stagesIndex, 0);
  p.connect(stagesIndex, 0, choose, 0);
  stages.forEach((stage, i) => p.connect(stage, 0, choose, i + 1));
  const band = choose;

  // Output: everything sums into one gain, then clip~ as a safety, to both sides.
  p.at(20, 600);
  p.comment("Output: Gain, then clip~ at +-1 so a loud setting cannot blow up the monitors.");
  const out = p.obj("*~ 0.");
  const gainR = p.obj("r ---gain");
  const gainLin = p.obj("expr pow(10.\\, $f1 / 20.)");
  p.connect(gainR, 0, gainLin, 0);
  p.connect(smoothed(p, gainLin), 0, out, 1);
  const clip = p.obj("clip~ -1. 1.");
  const plugout = p.obj("plugout~");
  p.connect(out, 0, clip, 0);
  p.connect(clip, 0, plugout, 0);
  p.connect(clip, 0, plugout, 1);

  // Hear Band: the unshifted Band, delayed by Lmax like the Copies.
  p.at(260, 360);
  p.comment("Hear Band: the Band itself, delayed by Lmax.");
  const bandDelay = p.obj(DELAY_LMAX);
  const bandGain = p.obj("*~ 0.");
  const bandR = p.obj("r ---band");
  p.connect(band, 0, bandDelay, 0);
  p.connect(bandDelay, 0, bandGain, 0);
  p.connect(smoothed(p, bandR), 0, bandGain, 1);
  p.connect(bandGain, 0, out, 0);

  // The engines, each behind a mute~, each output through a fade gain.
  const engines = [
    { name: "pitch-engine", build: pitchEngine, onExpr: "expr $i1 == 0", remeasure: true },
    { name: "freq-engine", build: freqEngine, onExpr: "expr $i1 > 0", remeasure: false },
  ].map((e, i) => {
    const x = 520 + i * 300;
    p.at(x, 100);
    const mute = p.obj("mute~");
    const sub = p.sub(e.name, e.build);
    p.connect(mute, 0, sub, 0);
    p.connect(band, 0, sub, 1);
    const fades = COPIES.map((_, k) => {
      p.at(x + k * 70, 200);
      const g = p.obj("*~ 0.");
      p.connect(sub, k, g, 0);
      return g;
    });
    return { ...e, mute, fades, x };
  });

  // Per Copy: both engines' outputs sum into its Level, then into the output.
  COPIES.forEach((_, k) => {
    const n = k + 1;
    p.at(520 + k * 200, 300);
    const level = p.obj("*~ 0.");
    for (const e of engines) p.connect(e.fades[k], 0, level, 0);
    const levelR = p.obj(`r ---c${n}_level`);
    const levelLin = p.obj(`expr ($f1 > ${MUTE_BELOW}) * pow(10.\\, $f1 / 20.)`);
    p.connect(levelR, 0, levelLin, 0);
    p.connect(smoothed(p, levelLin), 0, level, 1);
    p.connect(level, 0, out, 0);
  });

  const boxes = panel(p);

  // Per Copy: the Hz the frequency engine uses. Freq Hz takes the Copy's Hz dial; Pitch and
  // Freq match take centre * (2^(semitones / 12) - 1), so the Copy's centre lands where the pitch
  // engine puts it. The panel shows it either way.
  p.at(1300, 100);
  p.comment("Frequency engine Hz per Copy, and its readout.");
  const engineR = p.obj("r ---engine");
  const centreR = p.obj("r ---centre");
  COPIES.forEach((c, k) => {
    const n = k + 1;
    p.at(1300 + k * 260, 200);
    const all = p.obj(`pak 0 ${DEFAULT.centre}. ${c.semis}. ${c.hz}.`);
    p.connect(engineR, 0, all, 0);
    p.connect(centreR, 0, all, 1);
    p.connect(p.obj(`r ---c${n}_semis`), 0, all, 2);
    p.connect(p.obj(`r ---c${n}_hz`), 0, all, 3);
    const hz = p.obj("expr ($i1 == 2) * $f4 + ($i1 != 2) * $f2 * (pow(2.\\, $f3 / 12.) - 1.)");
    const both = p.obj("t f f");
    const send = p.obj(`s ---c${n}_fhz`);
    const format = p.obj(`sprintf set Copy ${n} freq shift: %.0f Hz`);
    const text = readout(p, `c${n}_readout`, `Copy ${n} freq shift: not set yet`, [114, 92 + k * 14, 180, 13]);
    p.connect(all, 0, hz, 0);
    p.connect(hz, 0, both, 0);
    p.connect(both, 1, send, 0);
    p.connect(both, 0, format, 0);
    p.connect(format, 0, text, 0);
  });

  // Pitch latency readout.
  p.at(1300, 400);
  const latR = p.obj("r ---ps_lat");
  const latFormat = p.obj("sprintf set PS latency: %ld smp");
  const latText = readout(p, "ps_readout", "PS latency: not measured yet", [162, 56, 132, 13]);
  p.connect(latR, 0, latFormat, 0);
  p.connect(latFormat, 0, latText, 0);

  // Engine switching.
  for (const e of engines) {
    engineSwitch(p, e.x, { engine: engineR, onExpr: e.onExpr, mute: e.mute, outputs: e.fades, remeasure: e.remeasure });
  }

  // Load: every control outputs its value, then latency is measured (again after a second).
  p.at(20, 460);
  const thisDevice = p.obj("live.thisdevice");
  const load = p.obj("t b b");
  p.connect(thisDevice, 0, load, 0);
  for (const b of boxes) p.connect(load, 1, b, 0);
  const defer = p.obj("deferlow");
  const measure = p.obj("s ---measure");
  const again = p.obj("delay 1000");
  const measureAgain = p.obj("s ---measure");
  p.connect(load, 0, defer, 0);
  p.connect(defer, 0, measure, 0);
  p.connect(defer, 0, again, 0);
  p.connect(again, 0, measureAgain, 0);

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

export function patcherText() {
  return JSON.stringify(buildPatcher(), null, 2) + "\n";
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const text = patcherText();
  fs.writeFileSync(new URL(`./${FILE}.maxpat`, import.meta.url), text);
  fs.writeFileSync(new URL(`./${FILE}.amxd`, import.meta.url), pack(text, AUDIO_EFFECT));
}
