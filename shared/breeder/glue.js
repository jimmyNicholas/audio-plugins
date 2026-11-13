// Breeder devices: the Max glue, the same for every drum. ES5 (Max 8's js object).
// Joined into each device's js by its build, after `var Engine = makeBreeder(...)`.
//
// Buttons may send 1 then 0, or a bang (as in TAP), so every press handler ignores a 0.
// Panel objects are found by their scripting names (varname) and set with "set", so nothing
// feeds back into this file. Each slot's drum lives in message boxes in the patch,
// <slot>_<part> for each of Engine.PARTS, so a MIDI note plays it without waiting for this
// file. Saved drums live in hidden stored-only numboxes, so they save with the set:
// save_<n>_<param> (0 to 1) and save_<n>_on.

var SLOTS = ["A", "B", "C"];
var SAVED = 4;
var PLAY_KEYS = ["C3", "D3", "E3"];
var SAVE_KEYS = ["F3", "G3", "A3", "B3"];
var NOUN = Engine.NOUN;
var state = null;
var heard = "A"; // the slot MIDI notes outside the mapped keys play: A, B, C or S1 to S4
var matchOn = true, showFav = false, shuffleOn = true;
var saved = [null, null, null, null]; // normalised values, or null when empty
var locks = {};
var voiceCache = {};

function named(name) { return this.patcher ? this.patcher.getnamed(name) : null; }
function show(name, text) { var o = named(name); if (o) o.message("set", text); }
function setBox(name, list) { var o = named(name); if (o) o.message.apply(o, ["set"].concat(list)); }
function pressed(v) { return v === undefined || v !== 0; }

function valuesOf(slot) {
  if (slot.charAt(0) === "S") return saved[Number(slot.charAt(1)) - 1];
  var c = state && state.candidates[SLOTS.indexOf(slot)];
  return c ? c.values : null;
}

function voiceOf(values) {
  var key = values.join(",") + (matchOn ? "m" : "r");
  if (!voiceCache[key]) voiceCache[key] = Engine.voice(values, matchOn);
  return voiceCache[key];
}

// Fill one slot's message boxes: <slot>_statics and one per envelope.
function fill(slot, values) {
  var v = voiceOf(values), i;
  for (i = 0; i < Engine.PARTS.length; i++) setBox(slot + "_" + Engine.PARTS[i], v[Engine.PARTS[i]]);
}

function render() {
  if (!state) return;
  var i, c, values;
  for (i = 0; i < 3; i++) {
    c = state.candidates[i];
    if (c) fill(SLOTS[i], c.values);
    show("fav_" + i, PLAY_KEYS[i] + (!c ? "  (none this round)" : (showFav && c.favourite ? "  favourite" : "")));
  }
  for (i = 0; i < SAVED; i++) {
    if (saved[i]) fill("S" + (i + 1), saved[i]);
    show("saved_" + i, SAVE_KEYS[i] + (saved[i] ? "  " + Engine.label(saved[i]) : "  empty"));
  }
  values = valuesOf(heard);
  if (!values) { heard = "A"; values = valuesOf(heard); }
  fill("CUR", values);
  show("status", "Round " + state.round + "  |  choices " + state.picks.length + "  |  variation " +
    state.step.toFixed(2) + "  |  hearing " + (heard.charAt(0) === "S" ? "saved " + heard.charAt(1) : heard));
  show("detail", Engine.describe(values));
  var slider = named("variation");
  if (slider) slider.message("set", state.step);
}

function init() {
  var i, j, o, values, on;
  for (i = 0; i < Engine.PARAMS.length; i++) {
    o = named("lock_" + i);
    if (o) locks[Engine.PARAMS[i].id] = o.getvalueof() === 1;
  }
  for (i = 0; i < SAVED; i++) {
    o = named("save_" + (i + 1) + "_on");
    on = o && o.getvalueof() === 1;
    if (!on) { saved[i] = null; continue; }
    values = [];
    for (j = 0; j < Engine.PARAMS.length; j++) {
      o = named("save_" + (i + 1) + "_" + j);
      values.push(o ? Number(o.getvalueof()) : 0.5);
    }
    saved[i] = values;
  }
  o = named("match"); if (o) matchOn = o.getvalueof() === 1;
  o = named("shuffle"); if (o) shuffleOn = o.getvalueof() === 1;
  o = named("showfav"); if (o) showFav = o.getvalueof() === 1;
  state = Engine.create(new Date().getTime() % 100000, { shuffle: shuffleOn, locks: locks });
  heard = "A";
  render();
}

// Hear a slot from the panel: it becomes the drum other notes play, and it plays now.
function play(slot) {
  if (!state || !valuesOf(slot)) { if (slot.charAt(0) === "S") show("status", "Saved " + slot.charAt(1) + " is empty."); return; }
  heard = slot;
  render();
  outlet(0, slot, 127);
}
function hear_a(v) { if (pressed(v)) play("A"); }
function hear_b(v) { if (pressed(v)) play("B"); }
function hear_c(v) { if (pressed(v)) play("C"); }
function play_1(v) { if (pressed(v)) play("S1"); }
function play_2(v) { if (pressed(v)) play("S2"); }
function play_3(v) { if (pressed(v)) play("S3"); }
function play_4(v) { if (pressed(v)) play("S4"); }

// A mapped key was played (C3 to E3, F3 to B3). The patch already played it; just remember it.
function heard_slot(slot) {
  if (!state || heard === slot || !valuesOf(slot)) return;
  heard = slot;
  render();
}

function choose(index, v) {
  if (!pressed(v) || !state || !state.candidates[index]) return;
  state = Engine.choose(state, index);
  heard = "A";
  render();
}
function choose_a(v) { choose(0, v); }
function choose_b(v) { choose(1, v); }
function choose_c(v) { choose(2, v); }

// Save the drum last heard into saved slot n (1 to 4), and into the set.
function save(n, v) {
  if (!pressed(v) || !state) return;
  var values = valuesOf(heard), j, o;
  if (!values) return;
  saved[n - 1] = values.slice();
  for (j = 0; j < values.length; j++) {
    o = named("save_" + n + "_" + j);
    if (o) o.message("set", values[j]);
  }
  o = named("save_" + n + "_on");
  if (o) o.message("set", 1);
  render();
  show("status", "Saved to " + n + " (" + SAVE_KEYS[n - 1] + "), after " + state.picks.length + " choices.");
}
function save_1(v) { save(1, v); }
function save_2(v) { save(2, v); }
function save_3(v) { save(3, v); }
function save_4(v) { save(4, v); }

function back(v) { if (pressed(v) && state) { state = Engine.back(state); heard = "A"; render(); } }
function reset(v) {
  if (!pressed(v) || !state) return;
  var i, o;
  for (i = 0; i < Engine.PARAMS.length; i++) {
    o = named("lock_" + i);
    if (o) o.message("set", 0);
  }
  locks = {};
  state = Engine.reset(state);
  heard = "A";
  render();
  show("status", "Reset: round 1, variation " + state.step.toFixed(2) + ", no locks. Saved " + NOUN + "s are kept.");
}
function breed_from_heard(v) {
  if (!pressed(v) || !state) return;
  state = Engine.breedFrom(state, valuesOf(heard));
  heard = "A";
  render();
}

// From the panel: Variation slider, locks and toggles.
function variation(value) { if (state) { state = Engine.setStep(state, value); render(); } }
function lock(index, value) {
  var id = Engine.PARAMS[index].id;
  locks[id] = value === 1;
  if (state) state = Engine.setLock(state, id, locks[id]);
}
function match(v) { matchOn = v === 1; render(); }
function shuffle(v) { shuffleOn = v === 1; if (state) state = Engine.setShuffle(state, shuffleOn); }
function showfav(v) { showFav = v === 1; render(); }
