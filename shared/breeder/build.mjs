// Builds a breeder device: the panel, the keys, the slots and the saved values are the same for
// every drum, so they live here. Each device's build-patch.mjs gives its name, its voice
// section (the MSP objects that make the sound) and its voice file.
//
// Plain MSP, live.* objects and one js object: no gen~, no packages. The js works out each
// drum's envelopes as line~ breakpoint lists and stores them in message boxes, so a MIDI note
// only bangs those boxes: nothing on the note path waits for js.
//
// The device's js is joined from shared/breeder/dsp.js, engine.js and glue.js and the device's
// voice file, so the device still loads one js file that sits next to its .amxd.

import fs from "node:fs";
import { pack, INSTRUMENT } from "../../tools/amxd.mjs";
import { Patch } from "./patch.mjs";

const APPVERSION = { major: 8, minor: 5, revision: 4, architecture: "x64", modernui: 1 };
export const DEVICE_HEIGHT = 169;
export const SAVED = 4;

// MIDI note numbers, with Live's names (C3 = 60).
export const PLAY_KEYS = { A: 60, B: 62, C: 64, S1: 65, S2: 67, S3: 69, S4: 71 };
export const ACTION_KEYS = { choose_a: 48, choose_b: 50, choose_c: 52, save_1: 53, save_2: 55, save_3: 57, save_4: 59 };
export const SLOTS = [...Object.keys(PLAY_KEYS), "CUR"];

// ---- panel controls -----------------------------------------------------------------------

export const UNIT = { float: 1, enum: 9 };

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

// ---- the device -----------------------------------------------------------------------------

// config:
//   title       "Kick Breeder": the device is "<title> Spike"
//   noun        "kick": as in "the kick you last heard"
//   jsFile      the joined js the device loads, next to the .amxd
//   paramNames  the short names of the engine's settings, in PARAMS order (lock labels)
//   parts       what each slot stores, in the order its trigger fires them: "statics" (the
//               settings list) first, then one line~ envelope each
//   width       the panel width
//   voice(p)    builds the voice; returns { inputs: { part: box }, resets: [box],
//               velocity: box }. Each part's store feeds inlet 0 of its input. Resets are
//               banged after the envelopes start (an oscillator's phase, say); velocity gets
//               the note's velocity.
function build(config, messages) {
  const { title, noun, jsFile, paramNames, parts } = config;
  const p = new Patch();

  const toJs = (control, prefix, js) => {
    messages.add(prefix.split(" ")[0]);
    const pre = p.obj(`prepend ${prefix}`);
    p.connect(control, 0, pre, 0);
    p.connect(pre, 0, js, 0);
  };
  const msgToJs = (text, js) => {
    messages.add(text.split(" ")[0]);
    const m = p.msg(text);
    p.connect(m, 0, js, 0);
    return m;
  };

  p.at(20, 20);
  p.comment(`${title} spike. Throwaway. See README.md. The js keeps the breeding state and fills each slot's message boxes.`);
  const js = p.obj(`js ${jsFile}`, { saved_object_attributes: { filename: jsFile, parameter_enable: 0 } });

  // Load: once the device is ready, the js reads the panel and the saved drums.
  p.at(20, 80);
  const thisDevice = p.obj("live.thisdevice");
  const defer = p.obj("deferlow");
  const init = msgToJs("init", js);
  p.connect(thisDevice, 0, defer, 0);
  p.connect(defer, 0, init, 0);

  // ---- panel ----
  // Column 1: title, readouts, toggles, Reset, Back, Breed from heard, key help.
  p.at(900, 20);
  label(p, `${title} spike`.toUpperCase(), [4, 2, 230, 16], { fontsize: 11 });
  readout(p, "status", [4, 20, 230, 24], "Loading...");
  readout(p, "detail", [4, 46, 230, 36]);
  const match = toggle(p, "Match Loudness", "match", "Match level", [4, 86, 74, 15], 1);
  const shuffle = toggle(p, "Shuffle Slots", "shuffle", "Shuffle", [82, 86, 74, 15], 1);
  const showFav = toggle(p, "Show Favourite", "showfav", "Show fav", [160, 86, 74, 15], 0);
  const reset = button(p, "Reset", "reset", [4, 106, 74, 15]);
  const back = button(p, "Back", "back", [82, 106, 74, 15]);
  const breedFrom = button(p, "Breed From Heard", "breed_from_heard", [160, 106, 74, 15], "Breed this");
  label(p, `Keys: C3 D3 E3 play A B C. F3 G3 A3 B3 play saved 1 to 4. C2 D2 E2 choose A B C. F2 G2 A2 B2 save. Other notes play the ${noun} you last heard.`, [4, 124, 230, 44], { linecount: 4, fontsize: 9 });

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

  // Column 3: saved drums.
  p.at(1300, 20);
  label(p, "SAVED", [392, 2, 60, 14]);
  for (let n = 1; n <= SAVED; n++) {
    const y = 18 + (n - 1) * 30;
    label(p, String(n), [392, y, 12, 15]);
    jsControls.push([button(p, `Play ${n}`, `play_${n}`, [406, y, 46, 15], "Play"), `play_${n}`]);
    jsControls.push([button(p, `Save ${n}`, `save_${n}`, [456, y, 46, 15], "Save"), `save_${n}`]);
    readout(p, `saved_${n - 1}`, [406, y + 16, 154, 13]);
  }
  label(p, `Save stores the ${noun} you last heard, with the set.`, [392, 140, 168, 26], { linecount: 2, fontsize: 9 });

  // Column 4: Variation slider and locks, three to a row.
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
  const locks = paramNames.map((name, i) =>
    toggle(p, `Lock ${name}`, `lock_${i}`, name, [612 + (i % 3) * 46, 18 + Math.floor(i / 3) * 18, 44, 15], 0));
  const lockRows = Math.ceil(paramNames.length / 3);
  label(p, "Lit: kept as it is in new variations.", [612, 22 + lockRows * 18, 136, 26], { linecount: 2, fontsize: 9 });

  // Hidden, stored-only values for the saved drums, read and written by the js.
  p.at(1700, 600);
  for (let n = 1; n <= SAVED; n++) {
    parameterBox(p, "live.numbox", `Saved ${n} On`, [0, 0, 40, 15], enumOf(["off", "on"], 0), { varname: `save_${n}_on`, hidden: true });
    paramNames.forEach((name, i) => {
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
    toJs(control, message, js);
  }
  toJs(match, "match", js);
  toJs(shuffle, "shuffle", js);
  toJs(showFav, "showfav", js);
  toJs(variation, "variation", js);
  locks.forEach((l, i) => toJs(l, `lock ${i}`, js));

  // ---- the voice: one monophonic drum, shared by every slot ----
  // A new hit restarts every envelope, like a drum machine's channel.
  const voice = config.voice(p);

  // ---- slots: each has its stores and a trigger ----
  // Trigger, right to left: tell the js which slot was heard (mapped keys only), each part
  // (settings first, then the envelopes), the resets, then velocity.
  const r = voice.resets.length;
  const trigText = "t f" + " b".repeat(r + parts.length + 1);
  const triggers = {};
  SLOTS.forEach((slot, k) => {
    p.at(20 + k * 190, 400);
    p.comment(`Slot ${slot}`);
    const note = p.obj("sel 0");
    const trig = p.obj(trigText);
    p.connect(note, 1, trig, 0);
    const stores = {};
    for (const part of parts) stores[part] = store(p, `${slot}_${part}`);
    for (const part of parts) p.connect(stores[part], 0, voice.inputs[part], 0);
    if (slot !== "CUR") p.connect(trig, r + parts.length + 1, msgToJs(`heard_slot ${slot}`, js), 0);
    parts.forEach((part, i) => p.connect(trig, r + parts.length - i, stores[part], 0));
    voice.resets.forEach((box, j) => p.connect(trig, r - j, box, 0));
    p.connect(trig, 0, voice.velocity, 0);
    triggers[slot] = note;
  });

  // Hear and Play buttons: the js sends "<slot> 127".
  p.at(20, 300);
  const fromJs = p.obj(`route ${SLOTS.join(" ")}`);
  p.connect(js, 0, fromJs, 0);
  SLOTS.forEach((slot, k) => p.connect(fromJs, k, triggers[slot], 0));

  // ---- MIDI in: routed by pitch ----
  p.at(20, 160);
  p.comment(`Notes by pitch (Live's names, C3 = 60). Play keys go straight to their slot; action keys go to the js; any other note plays the ${noun} last heard. Note off (velocity 0) is ignored.`);
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
    p.connect(noteOn, 1, msgToJs(message, js), 0);
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

  const width = config.width;
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

const SHARED_JS = ["dsp.js", "engine.js"];
const read = (url) => fs.readFileSync(url, "utf8").replace(/\s+$/, "") + "\n";

// The js the device loads: shared maths and engine, the voice, then the Max glue.
function joinJs(config, dir) {
  const shared = (name) => read(new URL(`./${name}`, import.meta.url));
  return [
    `// ${config.title} spike: GENERATED by \`npm run build\` from ${config.voiceFile} (the ${config.noun} voice)`,
    "// and shared/breeder/dsp.js, engine.js and glue.js. Do not edit this file; edit those.",
    "// ES5 only: Max 8's js object. Engine (makeBreeder) is pure and is unit tested in Node.",
    "",
    "inlets = 1;",
    "outlets = 1;",
    "// outlet 0: \"<slot> <velocity>\" plays a slot (A, B, C, S1 to S4 or CUR) from a panel button.",
    "",
    ...SHARED_JS.map(shared),
    read(new URL(`./${config.voiceFile}`, dir)),
    "var Engine = makeBreeder(Drum);",
    "if (typeof module !== \"undefined\" && module.exports) module.exports = Engine;",
    "",
    shared("glue.js"),
  ].join("\n");
}

// config as for build, plus file (the device's name) and voiceFile. dir: the device folder's
// URL. Returns what the device's build-patch.mjs exports and its tests use.
export function breederDevice(config, dir) {
  const messages = new Set();
  const patcherText = () => {
    messages.clear();
    return JSON.stringify(build(config, messages), null, 2) + "\n";
  };
  const jsText = () => joinJs(config, dir);
  const write = () => {
    const text = patcherText();
    fs.writeFileSync(new URL(`./${config.jsFile}`, dir), jsText());
    fs.writeFileSync(new URL(`./${config.file}.maxpat`, dir), text);
    fs.writeFileSync(new URL(`./${config.file}.amxd`, dir), pack(text, INSTRUMENT));
  };
  return { MESSAGES: messages, patcherText, jsText, write };
}
