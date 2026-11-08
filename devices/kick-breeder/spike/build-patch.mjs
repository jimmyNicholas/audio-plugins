// Generates the Kick Breeder spike device, so the patcher JSON stays consistent and reviewable.
//
//   node build-patch.mjs   (or npm run build)   writes "Kick Breeder Spike.maxpat" and packs
//                                                "Kick Breeder Spike.amxd" as an instrument
//
// Plain MSP, live.* objects and one js object: no gen~, no packages. The js works out each
// kick's envelopes as line~ breakpoint lists and stores them in message boxes, so a MIDI note
// only bangs those boxes: nothing on the note path waits for js.
//
// The Patch helper is a trimmed copy of the one in devices/slush-wave/prototype/.../build-patch.mjs.
// If a third device needs it, move it to tools/.

import fs from "node:fs";
import { fileURLToPath } from "node:url";
import { pack, INSTRUMENT } from "../../../tools/amxd.mjs";

export const FILE = "Kick Breeder Spike";
export const JS_FILE = "kick-breeder.js";
const APPVERSION = { major: 8, minor: 5, revision: 4, architecture: "x64", modernui: 1 };
const DEVICE_HEIGHT = 169;

// Inlet and outlet counts for every object used, so the tests can check every line.
// Max recomputes them on load; these only need to match what Max 8 builds.
function shape(text) {
  const [name, ...args] = text.split(" ");
  const sig = (n) => Array(n).fill("signal");
  const any = (n) => Array(n).fill("");
  switch (name) {
    case "midiin": return [1, ["int"]];
    case "midiparse": return [1, ["", "", "", "int", "int", "", "int"]];
    case "unpack": return [1, args.map((a) => (a.includes(".") ? "float" : "int"))];
    case "sel": return [2, ["bang", ""]];
    case "t": return [1, args.map((a) => ({ b: "bang", i: "int", f: "float" })[a] || "")];
    case "/": return [2, ["float"]];
    case "js": return [1, any(1)];
    case "route": return [2, any(args.length + 1)];
    case "prepend": return [1, any(1)];
    case "live.thisdevice": return [1, ["bang", "int", "int"]];
    case "deferlow": return [1, any(1)];
    case "line~": return [2, ["signal", "bang"]];
    case "cycle~": return [2, sig(1)];
    case "*~": case "+~": return [2, sig(1)];
    case "noise~": return [1, sig(1)];
    case "svf~": return [3, sig(4)];
    case "tanh~": return [1, sig(1)];
    case "lores~": return [3, sig(1)];
    case "clip~": return [3, sig(1)];
    case "plugout~": return [2, []];
    default: throw new Error("No shape for object: " + text);
  }
}

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

  obj(text, extra = {}) {
    const [numinlets, outlettype] = shape(text);
    return this.add({
      maxclass: "newobj",
      numinlets,
      numoutlets: outlettype.length,
      outlettype,
      patching_rect: this.place(Math.max(40, text.length * 6 + 10)),
      text,
      ...extra,
    });
  }

  msg(text = "") {
    return this.add({
      maxclass: "message",
      numinlets: 2,
      numoutlets: 1,
      outlettype: [""],
      patching_rect: this.place(Math.max(60, text.length * 6 + 10)),
      text,
    });
  }

  comment(text) {
    return this.add({
      maxclass: "comment",
      numinlets: 1,
      numoutlets: 0,
      patching_rect: this.place(Math.min(420, text.length * 6 + 10)),
      text,
    });
  }

  connect(from, outlet, to, inlet) {
    this.lines.push({ patchline: { destination: [to, inlet], source: [from, outlet] } });
  }
}

// ---- panel controls -----------------------------------------------------------------------

const UNIT = { float: 1, enum: 9 };

function parameterBox(p, maxclass, longname, rect, valueof, extra = {}) {
  const outlets = {
    "live.dial": ["", "float"],
    "live.slider": ["", "float"],
    "live.numbox": ["", "float"],
    "live.text": ["", ""],
  }[maxclass];
  const { shortname, hidden, ...rest } = extra;
  const box = {
    maxclass,
    varname: rest.varname || longname.toLowerCase().replace(/ /g, "_"),
    numinlets: 1,
    numoutlets: outlets.length,
    outlettype: outlets,
    parameter_enable: 1,
    ...rest,
    patching_rect: p.place(rect[2], rect[3]),
    saved_attribute_attributes: {
      valueof: {
        parameter_longname: longname,
        parameter_shortname: shortname || longname,
        parameter_invisible: 1, // Stored Only: saved with the Set, not automatable
        parameter_initial_enable: 1,
        ...valueof,
      },
    },
  };
  if (!hidden) {
    box.presentation = 1;
    box.presentation_rect = rect;
  }
  return p.add(box);
}

const enumOf = (items, initial) => ({
  parameter_type: 2,
  parameter_mmin: 0,
  parameter_mmax: items.length - 1,
  parameter_initial: [initial],
  parameter_unitstyle: UNIT.enum,
  parameter_enum: items,
});

// A press button: live.text in button mode.
function button(p, longname, varname, rect, text = longname) {
  return parameterBox(p, "live.text", longname, rect, enumOf(["off", "on"], 0), { varname, mode: 0, text, texton: text });
}

function toggle(p, longname, varname, text, rect, initial) {
  return parameterBox(p, "live.text", longname, rect, enumOf(["off", "on"], initial), { varname, mode: 1, text, texton: text });
}

function label(p, text, rect, extra = {}) {
  return p.add({
    maxclass: "live.comment",
    numinlets: 1,
    numoutlets: 0,
    patching_rect: p.place(rect[2], rect[3]),
    presentation: 1,
    presentation_rect: rect,
    text,
    ...extra,
  });
}

// A text readout the js fills in by scripting name.
function readout(p, varname, rect, text = "") {
  return p.add({
    maxclass: "comment",
    varname,
    fontname: "Arial",
    fontsize: 9,
    linecount: Math.max(1, Math.round(rect[3] / 11)),
    numinlets: 1,
    numoutlets: 0,
    patching_rect: p.place(rect[2], rect[3]),
    presentation: 1,
    presentation_rect: rect,
    text,
  });
}

// A message box the js fills with "set ..." by scripting name.
function store(p, varname) {
  return p.add({
    maxclass: "message",
    varname,
    numinlets: 2,
    numoutlets: 1,
    outlettype: [""],
    patching_rect: p.place(160),
    text: "",
  });
}

// Engine parameters, in KickBreeder.PARAMS order (kick-breeder.js): short names for the locks.
export const PARAM_NAMES = [
  "Pitch", "Sweep", "Sweep T", "Curve", "Decay", "Click", "Click T",
  "Noise", "Noise D", "Noise T", "Comp", "Sat", "Tone",
];
export const SAVED = 4;

// MIDI note numbers, with Live's names (C3 = 60).
export const PLAY_KEYS = { A: 60, B: 62, C: 64, S1: 65, S2: 67, S3: 69, S4: 71 };
export const ACTION_KEYS = { choose_a: 48, choose_b: 50, choose_c: 52, save_1: 53, save_2: 55, save_3: 57, save_4: 59 };
export const SLOTS = [...Object.keys(PLAY_KEYS), "CUR"];

// Every message the patch sends to the js, so the tests can check each has a function.
export const MESSAGES = new Set();

function toJs(p, control, prefix, js) {
  MESSAGES.add(prefix.split(" ")[0]);
  const pre = p.obj(`prepend ${prefix}`);
  p.connect(control, 0, pre, 0);
  p.connect(pre, 0, js, 0);
}

function msgToJs(p, text, js) {
  MESSAGES.add(text.split(" ")[0]);
  const m = p.msg(text);
  p.connect(m, 0, js, 0);
  return m;
}

// ---- the device -----------------------------------------------------------------------------

function build() {
  const p = new Patch();

  p.at(20, 20);
  p.comment("Kick Breeder spike. Throwaway. See README.md. The js keeps the breeding state and fills each slot's message boxes.");
  const js = p.obj(`js ${JS_FILE}`, { saved_object_attributes: { filename: JS_FILE, parameter_enable: 0 } });

  // Load: once the device is ready, the js reads the panel and the saved kicks.
  p.at(20, 80);
  const thisDevice = p.obj("live.thisdevice");
  const defer = p.obj("deferlow");
  const init = msgToJs(p, "init", js);
  p.connect(thisDevice, 0, defer, 0);
  p.connect(defer, 0, init, 0);

  // ---- panel ----
  // Column 1: title, readouts, toggles, Reset, Back, Breed from heard, key help.
  p.at(900, 20);
  label(p, "KICK BREEDER SPIKE", [4, 2, 230, 16], { fontsize: 11 });
  readout(p, "status", [4, 20, 230, 24], "Loading...");
  readout(p, "detail", [4, 46, 230, 36]);
  const match = toggle(p, "Match Loudness", "match", "Match level", [4, 86, 74, 15], 1);
  const shuffle = toggle(p, "Shuffle Slots", "shuffle", "Shuffle", [82, 86, 74, 15], 1);
  const showFav = toggle(p, "Show Favourite", "showfav", "Show fav", [160, 86, 74, 15], 0);
  const reset = button(p, "Reset", "reset", [4, 106, 74, 15]);
  const back = button(p, "Back", "back", [82, 106, 74, 15]);
  const breedFrom = button(p, "Breed From Heard", "breed_from_heard", [160, 106, 74, 15], "Breed this");
  label(p, "Keys: C3 D3 E3 play A B C. F3 G3 A3 B3 play saved 1 to 4. C2 D2 E2 choose A B C. F2 G2 A2 B2 save. Other notes play the kick you last heard.", [4, 124, 230, 44], { linecount: 4, fontsize: 9 });

  // Column 2: the slots.
  p.at(1150, 20);
  const jsControls = [];
  ["A", "B", "C"].forEach((letter, i) => {
    const y = 2 + i * 34;
    const l = letter.toLowerCase();
    label(p, letter, [242, y, 18, 18], { fontsize: 14 });
    jsControls.push([button(p, `Hear ${letter}`, `hear_${l}`, [262, y, 56, 15], "Hear"), `hear_${l}`]);
    jsControls.push([button(p, `Choose ${letter}`, `choose_${l}`, [322, y, 60, 15], "Choose"), `choose_${l}`]);
    readout(p, `fav_${i}`, [262, y + 17, 124, 13]);
  });
  label(p, "Choosing the favourite again makes the next variations closer.", [242, 106, 144, 26], { linecount: 2, fontsize: 9 });

  // Column 3: saved kicks.
  p.at(1300, 20);
  label(p, "SAVED", [392, 2, 60, 14]);
  for (let n = 1; n <= SAVED; n++) {
    const y = 18 + (n - 1) * 30;
    label(p, String(n), [392, y, 12, 15]);
    jsControls.push([button(p, `Play ${n}`, `play_${n}`, [406, y, 46, 15], "Play"), `play_${n}`]);
    jsControls.push([button(p, `Save ${n}`, `save_${n}`, [456, y, 46, 15], "Save"), `save_${n}`]);
    readout(p, `saved_${n - 1}`, [406, y + 16, 154, 13]);
  }
  label(p, "Save stores the kick you last heard, with the set.", [392, 140, 168, 26], { linecount: 2, fontsize: 9 });

  // Column 4: Variation slider and locks.
  p.at(1450, 20);
  label(p, "Variation", [566, 2, 50, 14]);
  const variation = parameterBox(p, "live.slider", "Variation", [570, 18, 30, 140], {
    parameter_type: 0,
    parameter_mmin: 0.02,
    parameter_mmax: 0.5,
    parameter_initial: [0.25],
    parameter_unitstyle: UNIT.float,
  }, { varname: "variation" });
  label(p, "LOCKS", [612, 2, 60, 14]);
  const locks = PARAM_NAMES.map((name, i) =>
    toggle(p, `Lock ${name}`, `lock_${i}`, name, [612 + (i % 3) * 46, 18 + Math.floor(i / 3) * 18, 44, 15], 0));
  label(p, "Lit: kept as it is in new variations.", [612, 112, 136, 26], { linecount: 2, fontsize: 9 });

  // Hidden, stored-only values for the saved kicks, read and written by the js.
  p.at(1700, 600);
  for (let n = 1; n <= SAVED; n++) {
    parameterBox(p, "live.numbox", `Saved ${n} On`, [0, 0, 40, 15], enumOf(["off", "on"], 0), { varname: `save_${n}_on`, hidden: true });
    PARAM_NAMES.forEach((name, i) => {
      parameterBox(p, "live.numbox", `Saved ${n} ${name}`, [0, 0, 40, 15], {
        parameter_type: 0,
        parameter_mmin: 0,
        parameter_mmax: 1,
        parameter_initial: [0.5],
        parameter_unitstyle: UNIT.float,
      }, { varname: `save_${n}_${i}`, hidden: true });
    });
  }

  // Panel to js.
  p.at(1700, 20);
  for (const [control, message] of [...jsControls, [reset, "reset"], [back, "back"], [breedFrom, "breed_from_heard"]]) {
    toJs(p, control, message, js);
  }
  toJs(p, match, "match", js);
  toJs(p, shuffle, "shuffle", js);
  toJs(p, showFav, "showfav", js);
  toJs(p, variation, "variation", js);
  locks.forEach((l, i) => toJs(p, l, `lock ${i}`, js));

  // ---- the voice: one monophonic kick, shared by every slot ----
  // A new hit restarts every envelope, like a drum machine's kick channel.
  p.at(240, 1000);
  p.comment("Voice. Body: cycle~ swept by the pitch envelope, times the amp envelope.");
  const freqLine = p.obj("line~");
  const osc = p.obj("cycle~");
  const ampLine = p.obj("line~");
  const body = p.obj("*~");
  p.connect(freqLine, 0, osc, 0);
  p.connect(osc, 0, body, 0);
  p.connect(ampLine, 0, body, 1);

  p.at(520, 1000);
  p.comment("Click: band-passed noise. Noise layer: low-passed noise. Each times its envelope.");
  const noise = p.obj("noise~");
  const clickFilter = p.obj("svf~ 3000 0.3");
  const clickLine = p.obj("line~");
  const click = p.obj("*~");
  const noiseFilter = p.obj("svf~ 2000 0.");
  const noiseLine = p.obj("line~");
  const noiseLayer = p.obj("*~");
  p.connect(noise, 0, clickFilter, 0);
  p.connect(clickFilter, 2, click, 0);
  p.connect(clickLine, 0, click, 1);
  p.connect(noise, 0, noiseFilter, 0);
  p.connect(noiseFilter, 0, noiseLayer, 0);
  p.connect(noiseLine, 0, noiseLayer, 1);

  p.at(760, 1000);
  p.comment("Settings: click tone, noise tone, saturation in, saturation out, tone, gain.");
  const statics = p.obj("unpack 0. 0. 0. 0. 0. 0.");

  p.at(240, 1250);
  p.comment("Mix, saturation: tanh~(x * k) / tanh(k), tone: lores~, level-matched gain, velocity, then clip~ for safety.");
  const mix = p.obj("+~");
  const mixNoise = p.obj("+~");
  const satIn = p.obj("*~ 1.");
  const shaper = p.obj("tanh~");
  const satOut = p.obj("*~ 1.");
  const tone = p.obj("lores~ 8000 0.");
  const gain = p.obj("*~ 0.5");
  const velocity = p.obj("*~ 1.");
  const safety = p.obj("clip~ -1. 1.");
  const out = p.obj("plugout~");
  p.connect(body, 0, mix, 0);
  p.connect(click, 0, mix, 1);
  p.connect(mix, 0, mixNoise, 0);
  p.connect(noiseLayer, 0, mixNoise, 1);
  p.connect(mixNoise, 0, satIn, 0);
  p.connect(satIn, 0, shaper, 0);
  p.connect(shaper, 0, satOut, 0);
  p.connect(satOut, 0, tone, 0);
  p.connect(tone, 0, gain, 0);
  p.connect(gain, 0, velocity, 0);
  p.connect(velocity, 0, safety, 0);
  p.connect(safety, 0, out, 0);
  p.connect(safety, 0, out, 1);
  p.connect(statics, 0, clickFilter, 1);
  p.connect(statics, 1, noiseFilter, 1);
  p.connect(statics, 2, satIn, 1);
  p.connect(statics, 3, satOut, 1);
  p.connect(statics, 4, tone, 1);
  p.connect(statics, 5, gain, 1);
  const vel = p.obj("/ 127.");
  p.connect(vel, 0, velocity, 1);
  const phase = p.msg("0.75");
  p.connect(phase, 0, osc, 1);

  // ---- slots: each has its stores and a trigger ----
  // Trigger, right to left: tell the js which slot was heard (mapped keys only), settings,
  // pitch sweep, noise, amp and click envelopes, oscillator phase (cycle~ is a cosine; 0.75
  // starts it at zero), then velocity.
  const triggers = {};
  SLOTS.forEach((slot, k) => {
    p.at(20 + k * 190, 400);
    p.comment(`Slot ${slot}`);
    const note = p.obj("sel 0");
    const trig = p.obj("t f b b b b b b b");
    p.connect(note, 1, trig, 0);
    const stores = {};
    for (const part of ["statics", "freq", "noise", "amp", "click"]) stores[part] = store(p, `${slot}_${part}`);
    p.connect(stores.statics, 0, statics, 0);
    p.connect(stores.freq, 0, freqLine, 0);
    p.connect(stores.noise, 0, noiseLine, 0);
    p.connect(stores.amp, 0, ampLine, 0);
    p.connect(stores.click, 0, clickLine, 0);
    if (slot !== "CUR") p.connect(trig, 7, msgToJs(p, `heard_slot ${slot}`, js), 0);
    p.connect(trig, 6, stores.statics, 0);
    p.connect(trig, 5, stores.freq, 0);
    p.connect(trig, 4, stores.noise, 0);
    p.connect(trig, 3, stores.amp, 0);
    p.connect(trig, 2, stores.click, 0);
    p.connect(trig, 1, phase, 0);
    p.connect(trig, 0, vel, 0);
    triggers[slot] = note;
  });

  // Hear and Play buttons: the js sends "<slot> 127".
  p.at(20, 300);
  const fromJs = p.obj(`route ${SLOTS.join(" ")}`);
  p.connect(js, 0, fromJs, 0);
  SLOTS.forEach((slot, k) => p.connect(fromJs, k, triggers[slot], 0));

  // ---- MIDI in: routed by pitch ----
  p.at(20, 160);
  p.comment("Notes by pitch (Live's names, C3 = 60). Play keys go straight to their slot; action keys go to the js; any other note plays the kick last heard. Note off (velocity 0) is ignored.");
  const midi = p.obj("midiin");
  const parse = p.obj("midiparse");
  const playKeys = Object.values(PLAY_KEYS);
  const actionKeys = Object.values(ACTION_KEYS);
  const byKey = p.obj(`route ${[...playKeys, ...actionKeys].join(" ")}`);
  p.connect(midi, 0, parse, 0);
  p.connect(parse, 0, byKey, 0);
  Object.keys(PLAY_KEYS).forEach((slot, k) => p.connect(byKey, k, triggers[slot], 0));
  p.at(1000, 160);
  Object.keys(ACTION_KEYS).forEach((message, k) => {
    const noteOn = p.obj("sel 0");
    p.connect(byKey, playKeys.length + k, noteOn, 0);
    p.connect(noteOn, 1, msgToJs(p, message, js), 0);
  });
  p.at(500, 160);
  const other = p.obj("unpack 0 0");
  p.connect(byKey, playKeys.length + actionKeys.length, other, 0);
  p.connect(other, 1, triggers.CUR, 0);

  const parameters = { inherited_shortname: 1 };
  for (const { box } of p.boxes) {
    if (!box.saved_attribute_attributes) continue;
    const v = box.saved_attribute_attributes.valueof;
    parameters[box.id] = [v.parameter_longname, v.parameter_shortname, 0];
  }

  const width = 754;
  return {
    patcher: {
      fileversion: 1,
      appversion: APPVERSION,
      classnamespace: "box",
      rect: [100, 100, 1500, 1000],
      openrect: [0, 0, width, DEVICE_HEIGHT],
      devicewidth: width,
      openinpresentation: 1,
      default_fontsize: 10,
      default_fontname: "Arial Bold",
      gridsize: [8, 8],
      boxanimatetime: 500,
      latency: 0,
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
        amxdtype: 1768515945,
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
  MESSAGES.clear();
  return JSON.stringify(build(), null, 2) + "\n";
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const text = patcherText();
  fs.writeFileSync(new URL(`./${FILE}.maxpat`, import.meta.url), text);
  fs.writeFileSync(new URL(`./${FILE}.amxd`, import.meta.url), pack(text, INSTRUMENT));
}
