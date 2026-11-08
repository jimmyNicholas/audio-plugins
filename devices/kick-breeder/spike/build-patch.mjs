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
    case "js": return [1, any(5)];
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

const UNIT = { int: 0, float: 1, ms: 2, hz: 3, st: 7, enum: 9 };

function parameterBox(p, maxclass, longname, rect, valueof, extra = {}) {
  const outlets = {
    "live.dial": ["", "float"],
    "live.text": ["", ""],
    "live.tab": ["", "", "float"],
  }[maxclass];
  return p.add({
    maxclass,
    varname: extra.varname || longname.toLowerCase().replace(/ /g, "_"),
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
        parameter_shortname: extra.shortname || longname,
        parameter_invisible: 1, // Stored Only: saved with the Set, not automatable
        parameter_initial_enable: 1,
        ...valueof,
      },
    },
  });
}

const enumOf = (items, initial) => ({
  parameter_type: 2,
  parameter_mmin: 0,
  parameter_mmax: items.length - 1,
  parameter_initial: [initial],
  parameter_unitstyle: UNIT.enum,
  parameter_enum: items,
});

// A press button: live.text in button mode. Sends "<message> <value>" to the js.
function button(p, longname, message, rect) {
  return parameterBox(p, "live.text", longname, rect, enumOf(["off", "on"], 0), {
    varname: message,
    mode: 0,
    text: longname,
    texton: longname,
  });
}

function toggle(p, longname, varname, text, rect, initial) {
  return parameterBox(p, "live.text", longname, rect, enumOf(["off", "on"], initial), {
    varname,
    mode: 1,
    text,
    texton: text,
  });
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

// The Kept dials, in the engine's parameter order (KickBreeder.PARAMS in kick-breeder.js).
export const KEPT = [
  { longname: "Kept Pitch", shortname: "Pitch", min: 30, max: 120, initial: 55, unit: UNIT.hz, exponent: 2 },
  { longname: "Kept Sweep", shortname: "Sweep", min: 0, max: 48, initial: 24, unit: UNIT.st },
  { longname: "Kept Sweep Time", shortname: "Sweep T", min: 5, max: 200, initial: 40, unit: UNIT.ms, exponent: 3 },
  { longname: "Kept Sweep Curve", shortname: "Curve", min: 0, max: 1, initial: 0.5, unit: UNIT.float },
  { longname: "Kept Decay", shortname: "Decay", min: 50, max: 1500, initial: 400, unit: UNIT.ms, exponent: 3 },
  { longname: "Kept Click", shortname: "Click", min: 0, max: 1, initial: 0.3, unit: UNIT.float },
  { longname: "Kept Click Tone", shortname: "Click T", min: 500, max: 12000, initial: 3000, unit: UNIT.hz, exponent: 3 },
  { longname: "Kept Drive", shortname: "Drive", min: 0, max: 1, initial: 0.2, unit: UNIT.float },
  { longname: "Kept Tone", shortname: "Tone", min: 200, max: 16000, initial: 8000, unit: UNIT.hz, exponent: 3 },
];

// Every message the panel sends to the js, so the tests can check each has a function.
export const MESSAGES = new Set();

function toJs(p, control, prefix, js) {
  MESSAGES.add(prefix.split(" ")[0]);
  const pre = p.obj(`prepend ${prefix}`);
  p.connect(control, 0, pre, 0);
  p.connect(pre, 0, js, 0);
}

// ---- the device -----------------------------------------------------------------------------

function build() {
  const p = new Patch();

  p.at(20, 20);
  p.comment("Kick Breeder spike. Throwaway. See README.md. The js keeps the breeding state and works out each kick's envelopes.");
  const js = p.obj(`js ${JS_FILE}`, { saved_object_attributes: { filename: JS_FILE, parameter_enable: 0 } });

  // Load: once the device is ready, the js reads the panel and makes the first two kicks.
  p.at(20, 80);
  const thisDevice = p.obj("live.thisdevice");
  const defer = p.obj("deferlow");
  const init = p.msg("init");
  p.connect(thisDevice, 0, defer, 0);
  p.connect(defer, 0, init, 0);
  p.connect(init, 0, js, 0);

  // ---- panel ----
  // Column 1: title, readouts, mode and toggles.
  p.at(900, 20);
  label(p, "KICK BREEDER SPIKE", [4, 2, 230, 16], { fontsize: 11 });
  readout(p, "status", [4, 20, 230, 24], "Loading...");
  readout(p, "detail", [4, 46, 230, 24]);
  label(p, "Mode", [4, 76, 34, 16]);
  const mode = parameterBox(p, "live.tab", "Mode", [40, 76, 120, 16], enumOf(["Breed", "Kept"], 0), {
    varname: "mode",
    num_lines_patching: 1,
    num_lines_presentation: 1,
  });
  const match = toggle(p, "Match Loudness", "match", "Match level", [4, 100, 74, 15], 1);
  const shuffle = toggle(p, "Shuffle Slots", "shuffle", "Shuffle", [82, 100, 74, 15], 1);
  const showFav = toggle(p, "Show Favourite", "showfav", "Show fav", [160, 100, 74, 15], 0);
  const playKick = button(p, "Play", "play_kick", [4, 122, 74, 15]);
  label(p, "Hear plays a slot. MIDI notes play the slot you last heard, or the kept kick.", [4, 140, 230, 26], { linecount: 2, fontsize: 9 });

  // Column 2: the slots and the search controls.
  p.at(1150, 20);
  const slotControls = [];
  for (const [i, letter] of ["A", "B", "C"].entries()) {
    const y = 2 + i * 34;
    label(p, letter, [242, y, 18, 18], { fontsize: 14 });
    const hear = button(p, `Hear ${letter}`, `hear_${letter.toLowerCase()}`, [262, y, 56, 15]);
    const choose = button(p, `Choose ${letter}`, `choose_${letter.toLowerCase()}`, [322, y, 60, 15]);
    readout(p, `fav_${i}`, [262, y + 17, 120, 13]);
    slotControls.push([hear, `hear_${letter.toLowerCase()}`], [choose, `choose_${letter.toLowerCase()}`]);
  }
  const search = [
    [button(p, "Wider", "wider", [242, 106, 68, 15]), "wider"],
    [button(p, "Closer", "closer", [314, 106, 68, 15]), "closer"],
    [button(p, "Back", "back", [242, 126, 68, 15]), "back"],
    [button(p, "Fresh", "fresh", [314, 126, 68, 15]), "fresh"],
  ];

  // Column 3: the Kept kick's dials, each with a lock under it, then Keep.
  p.at(1400, 20);
  const dials = [];
  const locks = [];
  KEPT.forEach((k, i) => {
    const x = 392 + (i % 5) * 46;
    const y = i < 5 ? 2 : 64;
    const valueof = {
      parameter_type: 0,
      parameter_mmin: k.min,
      parameter_mmax: k.max,
      parameter_initial: [k.initial],
      parameter_unitstyle: k.unit,
      parameter_invisible: 0, // automatable: the kept kick is the instrument's sound
    };
    if (k.exponent) valueof.parameter_exponent = k.exponent;
    dials.push(parameterBox(p, "live.dial", k.longname, [x, y, 44, 44], valueof, { varname: `kept_${i}`, shortname: k.shortname }));
    locks.push(toggle(p, `Lock ${k.shortname}`, `lock_${i}`, "Lock", [x, y + 46, 44, 13], 0));
  });
  const keep = button(p, "Keep", "keep", [622, 64, 60, 30]);
  const breedFromKept = button(p, "Breed From Kept", "breed_from_kept", [622, 98, 60, 30]);
  label(p, "Kept kick: saved with the set. Locks stop a setting changing while you breed.", [392, 128, 290, 28], { linecount: 2, fontsize: 9 });

  // Panel to js.
  p.at(1700, 20);
  for (const [control, message] of [...slotControls, ...search, [playKick, "play_kick"], [keep, "keep"], [breedFromKept, "breed_from_kept"]]) {
    toJs(p, control, message, js);
  }
  toJs(p, match, "match", js);
  toJs(p, shuffle, "shuffle", js);
  toJs(p, showFav, "showfav", js);
  toJs(p, mode, "set_mode", js);
  dials.forEach((d, i) => toJs(p, d, `kept_value ${i}`, js));
  locks.forEach((l, i) => toJs(p, l, `lock ${i}`, js));

  // ---- MIDI in: a note on plays the kick ----
  p.at(20, 160);
  p.comment("Note on (velocity above 0) plays the kick. Note off is ignored: the kick always rings out.");
  const midi = p.obj("midiin");
  const parse = p.obj("midiparse");
  const note = p.obj("unpack 0 0");
  const noteOn = p.obj("sel 0");
  p.connect(midi, 0, parse, 0);
  p.connect(parse, 0, note, 0);
  p.connect(note, 1, noteOn, 0);

  // One trigger for MIDI and for the Hear and Play buttons (js outlet 4). Right to left: start
  // the pitch sweep, reset the oscillator phase, start the amp and click envelopes, set the
  // velocity gain.
  p.at(20, 320);
  p.comment("Trigger. Right to left: pitch sweep, oscillator phase, amp envelope, click envelope, velocity.");
  const trig = p.obj("t f b b b b");
  p.connect(noteOn, 1, trig, 0);
  p.connect(js, 4, trig, 0);
  const vel = p.obj("/ 127.");
  p.connect(trig, 0, vel, 0);

  // Envelope stores. The js sends "set <pairs>"; a bang plays the stored pairs into line~.
  p.at(240, 360);
  p.comment("Envelope stores, filled by the js: target, time (ms) pairs for line~.");
  const freqStore = p.msg();
  const ampStore = p.msg();
  const clickStore = p.msg();
  const phase = p.msg("0.75");
  p.connect(js, 0, freqStore, 0);
  p.connect(js, 1, ampStore, 0);
  p.connect(js, 2, clickStore, 0);
  p.connect(trig, 4, freqStore, 0);
  p.connect(trig, 3, phase, 0);
  p.connect(trig, 2, ampStore, 0);
  p.connect(trig, 1, clickStore, 0);

  // Body: a sine (cycle~ is a cosine; phase 0.75 starts it at zero) swept by the frequency
  // envelope, shaped by the amp envelope.
  p.at(240, 520);
  p.comment("Body: cycle~ swept by the pitch envelope, times the amp envelope.");
  const freqLine = p.obj("line~");
  const osc = p.obj("cycle~");
  const ampLine = p.obj("line~");
  const body = p.obj("*~");
  p.connect(freqStore, 0, freqLine, 0);
  p.connect(freqLine, 0, osc, 0);
  p.connect(phase, 0, osc, 1);
  p.connect(ampStore, 0, ampLine, 0);
  p.connect(osc, 0, body, 0);
  p.connect(ampLine, 0, body, 1);

  // Click: band-passed noise, times the click envelope.
  p.at(520, 520);
  p.comment("Click: noise~ band-passed by svf~, times the click envelope.");
  const noise = p.obj("noise~");
  const svf = p.obj("svf~ 3000 0.3");
  const clickLine = p.obj("line~");
  const click = p.obj("*~");
  p.connect(noise, 0, svf, 0);
  p.connect(svf, 2, click, 0);
  p.connect(clickStore, 0, clickLine, 0);
  p.connect(clickLine, 0, click, 1);

  // Settings that are not envelopes: click tone, drive in, drive out, tone, output gain.
  p.at(760, 360);
  p.comment("Click tone, drive in, drive out, tone, gain.");
  const statics = p.obj("unpack 0. 0. 0. 0. 0.");
  p.connect(js, 3, statics, 0);

  // Mix, drive (tanh, normalised so full scale stays full scale), tone, level, safety clip.
  p.at(240, 760);
  p.comment("Mix, drive: tanh~(x * k) / tanh(k), tone: lores~, level-matched gain, velocity, then clip~ for safety.");
  const mix = p.obj("+~");
  const driveIn = p.obj("*~ 1.");
  const shaper = p.obj("tanh~");
  const driveOut = p.obj("*~ 1.");
  const tone = p.obj("lores~ 8000 0.");
  const gain = p.obj("*~ 0.5");
  const velocity = p.obj("*~ 1.");
  const safety = p.obj("clip~ -1. 1.");
  const out = p.obj("plugout~");
  p.connect(body, 0, mix, 0);
  p.connect(click, 0, mix, 1);
  p.connect(mix, 0, driveIn, 0);
  p.connect(driveIn, 0, shaper, 0);
  p.connect(shaper, 0, driveOut, 0);
  p.connect(driveOut, 0, tone, 0);
  p.connect(tone, 0, gain, 0);
  p.connect(gain, 0, velocity, 0);
  p.connect(velocity, 0, safety, 0);
  p.connect(safety, 0, out, 0);
  p.connect(safety, 0, out, 1);
  p.connect(statics, 0, svf, 1);
  p.connect(statics, 1, driveIn, 1);
  p.connect(statics, 2, driveOut, 1);
  p.connect(statics, 3, tone, 1);
  p.connect(statics, 4, gain, 1);
  p.connect(vel, 0, velocity, 1);

  const parameters = { inherited_shortname: 1 };
  for (const { box } of p.boxes) {
    if (!box.saved_attribute_attributes) continue;
    const v = box.saved_attribute_attributes.valueof;
    parameters[box.id] = [v.parameter_longname, v.parameter_shortname, 0];
  }

  const width = 686;
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
