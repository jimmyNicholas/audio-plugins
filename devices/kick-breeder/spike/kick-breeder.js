// Kick Breeder spike: the breeding engine, the kick voice maths, and the Max glue.
// Max 8's js object only understands ES5, so: var, function, no arrows, no Math.tanh or
// Math.imul. The engine (KickBreeder) is pure and is unit tested in Node
// (tests/kick-breeder.test.js); the glue at the bottom talks to the patch.
//
// Throwaway spike: see README.md in this folder for what it is testing.

inlets = 1;
outlets = 5;
// outlet 0: frequency envelope, "set f t f t ..." for the line~ that drives cycle~ (Hz, ms)
// outlet 1: amplitude envelope, "set v t v t ..."
// outlet 2: click envelope, "set v t v t ..."
// outlet 3: list: click tone (Hz), drive in, drive out, tone (Hz), output gain
// outlet 4: velocity, to play the current kick (from a Hear button)

var KickBreeder = (function () {
  var PARAMS = [
    { id: "pitch", name: "Pitch", min: 30, max: 120, scale: "exp", unit: "Hz" },
    { id: "sweep", name: "Sweep", min: 0, max: 48, scale: "lin", unit: "st" },
    { id: "sweepTime", name: "Sweep time", min: 5, max: 200, scale: "exp", unit: "ms" },
    { id: "sweepCurve", name: "Sweep curve", min: 0, max: 1, scale: "lin", unit: "" },
    { id: "decay", name: "Decay", min: 50, max: 1500, scale: "exp", unit: "ms" },
    { id: "clickLevel", name: "Click level", min: 0, max: 1, scale: "lin", unit: "" },
    { id: "clickTone", name: "Click tone", min: 500, max: 12000, scale: "exp", unit: "Hz" },
    { id: "drive", name: "Drive", min: 0, max: 1, scale: "lin", unit: "" },
    { id: "tone", name: "Tone", min: 200, max: 16000, scale: "exp", unit: "Hz" }
  ];
  var START_STEP = 0.25, MIN_STEP = 0.02, MAX_STEP = 0.5, SHRINK = 0.6, GROW = 1.6;
  var MODULUS = 2147483647;
  var TARGET_RMS = 0.25, RAW_GAIN = 0.5, PEAK_LIMIT = 0.95;

  function clone(x) { return JSON.parse(JSON.stringify(x)); }

  // Park-Miller generator: the product stays below 2^53, so plain doubles are exact.
  function rand(s) { s.seed = (s.seed * 16807) % MODULUS; return (s.seed - 1) / (MODULUS - 1); }
  function gauss(s) {
    var u1 = Math.max(rand(s), 1e-9), u2 = rand(s);
    return Math.sqrt(-2 * Math.log(u1)) * Math.cos(2 * Math.PI * u2);
  }
  // Reflect off the ends, so values near an edge are not piled up on it.
  function fold(v) {
    if (v < 0) v = -v;
    if (v > 1) v = 2 - v;
    return Math.min(1, Math.max(0, v));
  }

  function randomValues(s) {
    var out = [], i;
    for (i = 0; i < PARAMS.length; i++) out.push(rand(s));
    return out;
  }
  function mutate(s, values) {
    var out = [], i;
    for (i = 0; i < PARAMS.length; i++) {
      out.push(s.locks[PARAMS[i].id] ? values[i] : fold(values[i] + gauss(s) * s.step));
    }
    return out;
  }
  function candidate(s, values, favourite) {
    var c = { id: s.nextId, values: values, favourite: favourite };
    s.nextId += 1;
    return c;
  }
  function shuffle(s, list) {
    var i, j, t;
    for (i = list.length - 1; i > 0; i--) {
      j = Math.floor(rand(s) * (i + 1));
      t = list[i]; list[i] = list[j]; list[j] = t;
    }
    return list;
  }
  function breed(s, favValues) {
    var list = [candidate(s, favValues, true), candidate(s, mutate(s, favValues), false), candidate(s, mutate(s, favValues), false)];
    return s.shuffle ? shuffle(s, list) : list;
  }
  function snapshot(state) { var c = clone(state); c.past = []; return c; }
  function favourite(state) {
    var i;
    for (i = 0; i < state.candidates.length; i++) if (state.candidates[i].favourite) return state.candidates[i];
    return null;
  }

  function create(seed, options) {
    var o = options || {};
    var s = {
      seed: (Math.abs(Math.floor(seed)) % (MODULUS - 1)) + 1,
      round: 1, step: START_STEP, locks: o.locks ? clone(o.locks) : {}, shuffle: o.shuffle !== false,
      nextId: 1, candidates: [], past: [], picks: []
    };
    s.candidates = [candidate(s, randomValues(s), false), candidate(s, randomValues(s), false)];
    return s;
  }

  // The user prefers slot `index`. It becomes the favourite and two variations are bred from it.
  // Choosing the favourite again means "close", so the variations get smaller.
  function choose(state, index) {
    var picked = state.candidates[index], s;
    if (!picked) return state;
    s = clone(state);
    s.past.push(snapshot(state));
    if (picked.favourite) s.step = Math.max(MIN_STEP, s.step * SHRINK);
    s.picks.push({ round: s.round, slot: index, favourite: picked.favourite, step: s.step });
    s.round += 1;
    s.candidates = breed(s, picked.values.slice());
    return s;
  }
  function resize(state, factor) {
    var fav = favourite(state), s;
    if (!fav) return state;
    s = clone(state);
    s.past.push(snapshot(state));
    s.step = Math.min(MAX_STEP, Math.max(MIN_STEP, s.step * factor));
    s.candidates = breed(s, fav.values.slice());
    return s;
  }
  function wider(state) { return resize(state, GROW); }
  function closer(state) { return resize(state, SHRINK); }
  function back(state) {
    var s;
    if (!state.past.length) return state;
    s = clone(state.past[state.past.length - 1]);
    s.past = clone(state.past.slice(0, -1));
    return s;
  }
  function fresh(state) {
    var r = { seed: state.seed };
    rand(r);
    return create(r.seed, { shuffle: state.shuffle, locks: state.locks });
  }
  function setLock(state, id, on) {
    var s = clone(state);
    s.locks[id] = !!on;
    return s;
  }
  function setShuffle(state, on) { var s = clone(state); s.shuffle = !!on; return s; }
  // Start a new search with `values` as the favourite (the kept kick, for later tweaks).
  function breedFrom(state, values) {
    var s = clone(state);
    s.past.push(snapshot(state));
    s.step = START_STEP / 2;
    s.round += 1;
    s.candidates = breed(s, values.slice());
    return s;
  }

  function toReal(param, v) {
    if (param.scale === "exp") return param.min * Math.pow(param.max / param.min, v);
    return param.min + (param.max - param.min) * v;
  }
  function fromReal(param, r) {
    var v;
    if (param.scale === "exp") v = Math.log(r / param.min) / Math.log(param.max / param.min);
    else v = (r - param.min) / (param.max - param.min);
    return Math.min(1, Math.max(0, v));
  }
  function realValues(values) {
    var out = {}, i;
    for (i = 0; i < PARAMS.length; i++) out[PARAMS[i].id] = toReal(PARAMS[i], values[i]);
    return out;
  }

  // ---- the kick voice -----------------------------------------------------------------------
  // The patch plays three line~ envelopes from these breakpoints, so the shapes are worked out
  // here, once per kick, and nothing on the note path needs js.

  function tanh(x) {
    if (x > 20) return 1;
    if (x < -20) return -1;
    var e = Math.exp(2 * x);
    return (e - 1) / (e + 1);
  }

  // Frequency in Hz: from pitch * 2^(sweep/12) down to pitch over sweepTime, falling as
  // (1 - x)^power, where power grows with Sweep curve (a snappier fall).
  function freqPoints(p) {
    var start = Math.min(p.pitch * Math.pow(2, p.sweep / 12), 16000);
    var power = 1 + p.sweepCurve * 6, n = 12, out = [start, 0], i, x;
    for (i = 1; i <= n; i++) {
      x = i / n;
      out.push(p.pitch + (start - p.pitch) * Math.pow(1 - x, power), p.sweepTime / n);
    }
    return out;
  }
  // Amplitude: 1.5 ms rise, then an exponential fall to -60 dB over Decay, then silence.
  function ampPoints(p) {
    var n = 16, out = [0, 0, 1, 1.5], i;
    for (i = 1; i <= n; i++) out.push(Math.exp(-6.9 * i / n), p.decay / n);
    out.push(0, 5);
    return out;
  }
  // Click: a burst of filtered noise falling with a 4 ms time constant.
  function clickPoints(p) {
    var c = p.clickLevel * 2, times = [2, 4, 8, 12, 20], out = [c, 0], last = 0, i;
    for (i = 0; i < times.length; i++) {
      out.push(c * Math.exp(-times[i] / 4), times[i] - last);
      last = times[i];
    }
    out.push(0, 10);
    return out;
  }
  function valueAt(points, ms) {
    var t = 0, i, v0 = points[0], v1, dt;
    for (i = 2; i < points.length; i += 2) {
      v1 = points[i]; dt = points[i + 1];
      if (ms <= t + dt) return dt > 0 ? v0 + (v1 - v0) * (ms - t) / dt : v1;
      t += dt; v0 = v1;
    }
    return v0;
  }

  // A rough simulation of the patch's voice, to measure loudness: the loudest 50 ms window of
  // RMS in the first 400 ms. Close enough to level-match candidates; it is not sample exact.
  function loudness(p) {
    var sr = 44100, len = Math.floor(sr * 0.4), win = Math.floor(sr * 0.05), hop = Math.floor(win / 4);
    var f = freqPoints(p), a = ampPoints(p), c = clickPoints(p);
    var k = 1 + p.drive * 12, outGain = 1 / tanh(k);
    var phase = 0.75, noise = 12345, low = 0, band = 0;
    // Chamberlin state-variable band-pass, kept below sr / 8 so it stays stable.
    var fc = 2 * Math.sin(Math.PI * Math.min(p.clickTone, sr / 8) / sr), q = 1.0;
    var lp = 1 - Math.exp(-2 * Math.PI * Math.min(p.tone, sr / 2.2) / sr), y = 0;
    var data = [], peak = 0, i, ms, n, x;
    for (i = 0; i < len; i++) {
      ms = i * 1000 / sr;
      phase += valueAt(f, ms) / sr;
      noise = (noise * 16807) % MODULUS;
      n = noise / MODULUS * 2 - 1;
      low += fc * band; band += fc * (n - low - q * band);
      x = Math.cos(2 * Math.PI * phase) * valueAt(a, ms) + band * valueAt(c, ms);
      x = tanh(k * x) * outGain;
      y += lp * (x - y);
      data.push(y);
      peak = Math.max(peak, Math.abs(y));
    }
    var best = 0, j, sum;
    for (i = 0; i + win <= len; i += hop) {
      sum = 0;
      for (j = i; j < i + win; j++) sum += data[j] * data[j];
      best = Math.max(best, Math.sqrt(sum / win));
    }
    return { rms: best, peak: peak };
  }

  // Everything the patch needs to play one kick. match: level-match it to the others.
  function voice(values, match) {
    var p = realValues(values), k = 1 + p.drive * 12, gain = RAW_GAIN, level;
    if (match) {
      level = loudness(p);
      gain = level.rms > 0 ? TARGET_RMS / level.rms : 1;
      if (level.peak * gain > PEAK_LIMIT) gain = PEAK_LIMIT / level.peak;
    }
    return {
      freq: freqPoints(p), amp: ampPoints(p), click: clickPoints(p),
      clickTone: p.clickTone, driveIn: k, driveOut: 1 / tanh(k), tone: p.tone, gain: gain
    };
  }

  function describe(values) {
    var p = realValues(values);
    function hz(v) { return v >= 1000 ? (v / 1000).toFixed(1) + " kHz" : v.toFixed(0) + " Hz"; }
    return hz(p.pitch) + ", sweep " + p.sweep.toFixed(0) + " st in " + p.sweepTime.toFixed(0) + " ms, decay " +
      p.decay.toFixed(0) + " ms, click " + p.clickLevel.toFixed(2) + ", drive " + p.drive.toFixed(2) + ", tone " + hz(p.tone);
  }

  return {
    PARAMS: PARAMS, START_STEP: START_STEP, MIN_STEP: MIN_STEP,
    create: create, choose: choose, wider: wider, closer: closer, back: back, fresh: fresh,
    setLock: setLock, setShuffle: setShuffle, breedFrom: breedFrom, favourite: favourite,
    toReal: toReal, fromReal: fromReal, realValues: realValues,
    voice: voice, loudness: loudness, valueAt: valueAt, describe: describe
  };
})();

if (typeof module !== "undefined" && module.exports) module.exports = KickBreeder;

// ---- Max glue -------------------------------------------------------------------------------
// Buttons may send 1 then 0, or a bang (as in TAP), so every press handler ignores a 0.
// Panel objects are found by their scripting names (varname) and set with "set", so nothing
// feeds back into this file.

var LETTERS = ["A", "B", "C"];
var MODE_BREED = 0, MODE_KEPT = 1;
var state = null;
var hearing = 0;
var mode = MODE_BREED;
var matchOn = true, showFav = false, shuffleOn = true;
var kept = []; // real values, read from the Kept dials
var locks = {};

function named(name) { return this.patcher ? this.patcher.getnamed(name) : null; }
function show(name, text) { var o = named(name); if (o) o.message("set", text); }
function pressed(v) { return v === undefined || v !== 0; }

function keptValues() {
  var out = [], i;
  for (i = 0; i < KickBreeder.PARAMS.length; i++) {
    var r = kept[i] === undefined ? KickBreeder.toReal(KickBreeder.PARAMS[i], 0.5) : kept[i];
    out.push(KickBreeder.fromReal(KickBreeder.PARAMS[i], r));
  }
  return out;
}

function currentValues() {
  if (mode === MODE_KEPT) return keptValues();
  var c = state && state.candidates[hearing];
  return c ? c.values : null;
}

function sendVoice() {
  var values = currentValues();
  if (!values) return;
  var v = KickBreeder.voice(values, matchOn);
  outlet(3, [v.clickTone, v.driveIn, v.driveOut, v.tone, v.gain]);
  outlet(2, ["set"].concat(v.click));
  outlet(1, ["set"].concat(v.amp));
  outlet(0, ["set"].concat(v.freq));
}

function render() {
  if (!state) return;
  var i, c, values = currentValues();
  if (mode === MODE_KEPT) {
    show("status", "Kept kick. MIDI plays it. Turn the dials to tweak, or Breed from kept.");
  } else {
    show("status", "Round " + state.round + "  |  choices " + state.picks.length + "  |  variation " +
      state.step.toFixed(2) + "  |  hearing " + LETTERS[hearing]);
  }
  show("detail", values ? KickBreeder.describe(values) : "");
  for (i = 0; i < 3; i++) {
    c = state.candidates[i];
    show("fav_" + i, !c ? "(none)" : (showFav && c.favourite ? "favourite" : ""));
  }
  sendVoice();
}

function init() {
  var i, o;
  for (i = 0; i < KickBreeder.PARAMS.length; i++) {
    o = named("kept_" + i);
    if (o) kept[i] = o.getvalueof();
    o = named("lock_" + i);
    if (o) locks[KickBreeder.PARAMS[i].id] = o.getvalueof() === 1;
  }
  o = named("match"); if (o) matchOn = o.getvalueof() === 1;
  o = named("shuffle"); if (o) shuffleOn = o.getvalueof() === 1;
  o = named("showfav"); if (o) showFav = o.getvalueof() === 1;
  o = named("mode"); if (o) mode = o.getvalueof();
  state = KickBreeder.create(new Date().getTime() % 100000, { shuffle: shuffleOn, locks: locks });
  hearing = 0;
  render();
}

function hear(slot) {
  if (!state || !state.candidates[slot]) return;
  if (mode === MODE_KEPT) { show("status", "Kept kick. Press Breed from kept to choose again."); return; }
  hearing = slot;
  render();
  outlet(4, 127);
}

function choose(slot, v) {
  if (!pressed(v) || !state || !state.candidates[slot]) return;
  if (mode === MODE_KEPT) { show("status", "Kept kick. Press Breed from kept to choose again."); return; }
  state = KickBreeder.choose(state, slot);
  hearing = 0;
  render();
}

function hear_a(v) { if (pressed(v)) hear(0); }
function hear_b(v) { if (pressed(v)) hear(1); }
function hear_c(v) { if (pressed(v)) hear(2); }
function choose_a(v) { choose(0, v); }
function choose_b(v) { choose(1, v); }
function choose_c(v) { choose(2, v); }
function play_kick(v) { if (pressed(v)) outlet(4, 127); }

function wider(v) { if (pressed(v) && state && mode === MODE_BREED) { state = KickBreeder.wider(state); hearing = 0; render(); } }
function closer(v) { if (pressed(v) && state && mode === MODE_BREED) { state = KickBreeder.closer(state); hearing = 0; render(); } }
function back(v) { if (pressed(v) && state && mode === MODE_BREED) { state = KickBreeder.back(state); hearing = 0; render(); } }
function fresh(v) { if (pressed(v) && state && mode === MODE_BREED) { state = KickBreeder.fresh(state); hearing = 0; render(); } }

function setMode(m) {
  mode = m;
  var o = named("mode");
  if (o) o.message("set", m);
}

// Copy the kick being heard into the Kept dials, and play the kept kick from now on.
function keep(v) {
  if (!pressed(v) || !state || mode === MODE_KEPT) return;
  var c = state.candidates[hearing], i, o, r;
  if (!c) return;
  for (i = 0; i < KickBreeder.PARAMS.length; i++) {
    r = KickBreeder.toReal(KickBreeder.PARAMS[i], c.values[i]);
    kept[i] = r;
    o = named("kept_" + i);
    if (o) o.message("set", r);
  }
  setMode(MODE_KEPT);
  render();
  show("status", "Kept after " + state.picks.length + " choices, in round " + state.round + ". MIDI plays it.");
}

function breed_from_kept(v) {
  if (!pressed(v) || !state) return;
  state = KickBreeder.breedFrom(state, keptValues());
  hearing = 0;
  setMode(MODE_BREED);
  render();
}

// From the panel: Kept dials, locks, toggles and the Mode tab.
function kept_value(index, value) { kept[index] = value; if (mode === MODE_KEPT) render(); }
function lock(index, value) {
  var id = KickBreeder.PARAMS[index].id;
  locks[id] = value === 1;
  if (state) state = KickBreeder.setLock(state, id, locks[id]);
}
function match(v) { matchOn = v === 1; render(); }
function shuffle(v) { shuffleOn = v === 1; if (state) state = KickBreeder.setShuffle(state, shuffleOn); }
function showfav(v) { showFav = v === 1; render(); }
function set_mode(v) { mode = v; render(); }
