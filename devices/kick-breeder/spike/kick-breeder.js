// Kick Breeder spike: the breeding engine, the kick voice maths, and the Max glue.
// Max 8's js object only understands ES5, so: var, function, no arrows, no Math.tanh or
// Math.imul. The engine (KickBreeder) is pure and is unit tested in Node
// (tests/engine.test.js); the glue at the bottom talks to the patch.
//
// Throwaway spike: see README.md in this folder for what it is testing.

inlets = 1;
outlets = 1;
// outlet 0: "<slot> <velocity>" plays a slot (A, B, C, S1 to S4 or CUR) from a panel button.
// Each slot's kick lives in message boxes in the patch, filled here by scripting name, so a
// MIDI note plays a kick without waiting for this file.

var KickBreeder = (function () {
  var PARAMS = [
    { id: "pitch", name: "Pitch", short: "Pitch", min: 30, max: 120, scale: "exp" },
    { id: "sweep", name: "Sweep", short: "Sweep", min: 0, max: 48, scale: "lin" },
    { id: "sweepTime", name: "Sweep time", short: "Sweep T", min: 5, max: 200, scale: "exp" },
    { id: "sweepCurve", name: "Sweep curve", short: "Curve", min: 0, max: 1, scale: "lin" },
    { id: "decay", name: "Decay", short: "Decay", min: 50, max: 1500, scale: "exp" },
    { id: "clickLevel", name: "Click level", short: "Click", min: 0, max: 1, scale: "lin" },
    { id: "clickTone", name: "Click tone", short: "Click T", min: 500, max: 12000, scale: "exp" },
    { id: "noiseLevel", name: "Noise level", short: "Noise", min: 0, max: 1, scale: "lin" },
    { id: "noiseDecay", name: "Noise decay", short: "Noise D", min: 10, max: 1000, scale: "exp" },
    { id: "noiseTone", name: "Noise tone", short: "Noise T", min: 200, max: 12000, scale: "exp" },
    { id: "compression", name: "Compression", short: "Comp", min: 0, max: 1, scale: "lin" },
    { id: "saturation", name: "Saturation", short: "Sat", min: 0, max: 1, scale: "lin" },
    { id: "tone", name: "Tone", short: "Tone", min: 200, max: 16000, scale: "exp" }
  ];
  var START_STEP = 0.25, MIN_STEP = 0.02, MAX_STEP = 0.5, SHRINK = 0.6;
  var MODULUS = 2147483647;
  var TARGET_RMS = 0.25, RAW_GAIN = 0.5, PEAK_LIMIT = 0.95;
  var COMP_THRESHOLD = -24; // dB below the envelope's peak

  function clone(x) { return JSON.parse(JSON.stringify(x)); }
  function clampStep(v) { return Math.min(MAX_STEP, Math.max(MIN_STEP, v)); }

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
    if (picked.favourite) s.step = clampStep(s.step * SHRINK);
    s.picks.push({ round: s.round, slot: index, favourite: picked.favourite, step: s.step });
    s.round += 1;
    s.candidates = breed(s, picked.values.slice());
    return s;
  }
  // The Variation slider: a new size, and the two variations bred again at that size.
  // Not added to Back's history, so dragging the slider does not fill it.
  function setStep(state, step) {
    var fav = favourite(state), s = clone(state);
    s.step = clampStep(step);
    if (fav) s.candidates = breed(s, fav.values.slice());
    return s;
  }
  function back(state) {
    var s;
    if (!state.past.length) return state;
    s = clone(state.past[state.past.length - 1]);
    s.past = clone(state.past.slice(0, -1));
    return s;
  }
  // Start again from round 1: new kicks, the starting variation size, and no locks.
  function reset(state) {
    var r = { seed: state.seed };
    rand(r);
    return create(r.seed, { shuffle: state.shuffle });
  }
  function setLock(state, id, on) {
    var s = clone(state);
    s.locks[id] = !!on;
    return s;
  }
  function setShuffle(state, on) { var s = clone(state); s.shuffle = !!on; return s; }
  // Start a new search with `values` as the favourite.
  function breedFrom(state, values) {
    var s = clone(state);
    s.past.push(snapshot(state));
    s.step = clampStep(START_STEP / 2);
    s.round += 1;
    s.candidates = breed(s, values.slice());
    return s;
  }

  function toReal(param, v) {
    if (param.scale === "exp") return param.min * Math.pow(param.max / param.min, v);
    return param.min + (param.max - param.min) * v;
  }
  function realValues(values) {
    var out = {}, i;
    for (i = 0; i < PARAMS.length; i++) out[PARAMS[i].id] = toReal(PARAMS[i], values[i]);
    return out;
  }

  // ---- the kick voice -----------------------------------------------------------------------
  // The patch plays four line~ envelopes from these breakpoints (target, time in ms), so the
  // shapes are worked out here, once per kick.

  function tanh(x) {
    if (x > 20) return 1;
    if (x < -20) return -1;
    var e = Math.exp(2 * x);
    return (e - 1) / (e + 1);
  }

  // Compression of a one-shot: a static compressor on the envelope. Above the threshold, level
  // in dB is divided by the ratio; then make-up gain brings the peak back to full. The tail
  // comes up against the hit, as when a kick is compressed. It works on the envelope, not the
  // audio, so it never pumps, and it leaves the click alone (like a slow attack).
  function compress(level, amount) {
    if (level <= 0) return 0;
    var ratio = 1 + amount * 9, t = COMP_THRESHOLD;
    var shape = function (db) { return db > t ? t + (db - t) / ratio : db; };
    var db = 20 * Math.log(level) / Math.LN10;
    return Math.pow(10, (shape(db) - shape(0)) / 20);
  }

  // Frequency in Hz: from pitch * 2^(sweep/12) down to pitch over Sweep time, falling as
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
  // A 1.5 ms rise, then an exponential fall to -60 dB over `length`, compressed, then silence.
  function decayPoints(peak, length, comp) {
    var n = 16, out = [0, 0, peak, 1.5], i;
    for (i = 1; i <= n; i++) out.push(peak * compress(Math.exp(-6.9 * i / n), comp), length / n);
    out.push(0, 5);
    return out;
  }
  function ampPoints(p) { return decayPoints(1, p.decay, p.compression); }
  function noisePoints(p) { return decayPoints(p.noiseLevel * 0.6, p.noiseDecay, p.compression); }
  // Click: a burst of band-passed noise falling with a 4 ms time constant.
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
  // Steps through breakpoints sample by sample, faster than valueAt for long runs.
  function stepper(points, msPerSample) {
    var i = 2, t = 0, v0 = points[0], ms = 0;
    return function () {
      while (i < points.length && ms > t + points[i + 1]) { t += points[i + 1]; v0 = points[i]; i += 2; }
      var out = i >= points.length ? v0 : (points[i + 1] > 0 ? v0 + (points[i] - v0) * (ms - t) / points[i + 1] : points[i]);
      ms += msPerSample;
      return out;
    };
  }

  // A rough simulation of the patch's voice, to measure loudness: the loudest 50 ms window of
  // RMS in the first 300 ms. Close enough to level-match kicks; it is not sample exact.
  function loudness(p) {
    var sr = 22050, len = Math.floor(sr * 0.3), win = Math.floor(sr * 0.05), hop = Math.floor(win / 4);
    var msPer = 1000 / sr;
    var f = stepper(freqPoints(p), msPer), a = stepper(ampPoints(p), msPer);
    var c = stepper(clickPoints(p), msPer), nz = stepper(noisePoints(p), msPer);
    var k = 1 + p.saturation * 12, outGain = 1 / tanh(k);
    var phase = 0.75, seed = 12345, low = 0, band = 0, nlow = 0;
    // Chamberlin state-variable band-pass for the click, kept below sr / 8 so it stays stable.
    var fc = 2 * Math.sin(Math.PI * Math.min(p.clickTone, sr / 8) / sr), q = 1.0;
    var nfc = 1 - Math.exp(-2 * Math.PI * Math.min(p.noiseTone, sr / 2.2) / sr);
    var lp = 1 - Math.exp(-2 * Math.PI * Math.min(p.tone, sr / 2.2) / sr), y = 0;
    var data = [], peak = 0, i, n, x;
    for (i = 0; i < len; i++) {
      phase += f() / sr;
      seed = (seed * 16807) % MODULUS;
      n = seed / MODULUS * 2 - 1;
      low += fc * band; band += fc * (n - low - q * band);
      nlow += nfc * (n - nlow);
      x = Math.cos(2 * Math.PI * phase) * a() + band * c() + nlow * nz();
      x = tanh(k * x) * outGain;
      y += lp * (x - y);
      data.push(y);
      if (Math.abs(y) > peak) peak = Math.abs(y);
    }
    var best = 0, j, sum;
    for (i = 0; i + win <= len; i += hop) {
      sum = 0;
      for (j = i; j < i + win; j++) sum += data[j] * data[j];
      if (sum > best) best = sum;
    }
    return { rms: Math.sqrt(best / win), peak: peak };
  }

  // Everything the patch needs to play one kick. match: level-match it to the others.
  function voice(values, match) {
    var p = realValues(values), k = 1 + p.saturation * 12, gain = RAW_GAIN, level;
    if (match) {
      level = loudness(p);
      gain = level.rms > 0 ? TARGET_RMS / level.rms : 1;
      if (level.peak * gain > PEAK_LIMIT) gain = PEAK_LIMIT / level.peak;
    }
    return {
      freq: freqPoints(p), amp: ampPoints(p), click: clickPoints(p), noise: noisePoints(p),
      // In the order the patch unpacks them: click tone, noise tone, saturation in and out,
      // tone, output gain.
      statics: [p.clickTone, p.noiseTone, k, 1 / tanh(k), p.tone, gain]
    };
  }

  function describe(values) {
    var p = realValues(values);
    function hz(v) { return v >= 1000 ? (v / 1000).toFixed(1) + " kHz" : v.toFixed(0) + " Hz"; }
    return hz(p.pitch) + ", sweep " + p.sweep.toFixed(0) + " st in " + p.sweepTime.toFixed(0) + " ms, decay " +
      p.decay.toFixed(0) + " ms, click " + p.clickLevel.toFixed(2) + ", noise " + p.noiseLevel.toFixed(2) +
      ", comp " + p.compression.toFixed(2) + ", sat " + p.saturation.toFixed(2) + ", tone " + hz(p.tone);
  }

  return {
    PARAMS: PARAMS, START_STEP: START_STEP, MIN_STEP: MIN_STEP, MAX_STEP: MAX_STEP,
    create: create, choose: choose, setStep: setStep, back: back, reset: reset,
    setLock: setLock, setShuffle: setShuffle, breedFrom: breedFrom, favourite: favourite,
    toReal: toReal, realValues: realValues, compress: compress,
    voice: voice, loudness: loudness, valueAt: valueAt, describe: describe
  };
})();

if (typeof module !== "undefined" && module.exports) module.exports = KickBreeder;

// ---- Max glue -------------------------------------------------------------------------------
// Buttons may send 1 then 0, or a bang (as in TAP), so every press handler ignores a 0.
// Panel objects are found by their scripting names (varname) and set with "set", so nothing
// feeds back into this file. Saved kicks live in hidden stored-only numboxes, so they save
// with the set: save_<n>_<param> (0 to 1) and save_<n>_on.

var SLOTS = ["A", "B", "C"];
var SAVED = 4;
var PLAY_KEYS = ["C3", "D3", "E3"];
var SAVE_KEYS = ["F3", "G3", "A3", "B3"];
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
  if (!voiceCache[key]) voiceCache[key] = KickBreeder.voice(values, matchOn);
  return voiceCache[key];
}

// Fill one slot's message boxes: <slot>_freq, _amp, _click, _noise and _statics.
function fill(slot, values) {
  var v = voiceOf(values);
  setBox(slot + "_statics", v.statics);
  setBox(slot + "_freq", v.freq);
  setBox(slot + "_noise", v.noise);
  setBox(slot + "_amp", v.amp);
  setBox(slot + "_click", v.click);
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
    show("saved_" + i, SAVE_KEYS[i] + (saved[i] ? "  " + KickBreeder.describe(saved[i]).split(",")[0] + " kick" : "  empty"));
  }
  values = valuesOf(heard);
  if (!values) { heard = "A"; values = valuesOf(heard); }
  fill("CUR", values);
  show("status", "Round " + state.round + "  |  choices " + state.picks.length + "  |  variation " +
    state.step.toFixed(2) + "  |  hearing " + (heard.charAt(0) === "S" ? "saved " + heard.charAt(1) : heard));
  show("detail", KickBreeder.describe(values));
  var slider = named("variation");
  if (slider) slider.message("set", state.step);
}

function init() {
  var i, j, o, values, on;
  for (i = 0; i < KickBreeder.PARAMS.length; i++) {
    o = named("lock_" + i);
    if (o) locks[KickBreeder.PARAMS[i].id] = o.getvalueof() === 1;
  }
  for (i = 0; i < SAVED; i++) {
    o = named("save_" + (i + 1) + "_on");
    on = o && o.getvalueof() === 1;
    if (!on) { saved[i] = null; continue; }
    values = [];
    for (j = 0; j < KickBreeder.PARAMS.length; j++) {
      o = named("save_" + (i + 1) + "_" + j);
      values.push(o ? Number(o.getvalueof()) : 0.5);
    }
    saved[i] = values;
  }
  o = named("match"); if (o) matchOn = o.getvalueof() === 1;
  o = named("shuffle"); if (o) shuffleOn = o.getvalueof() === 1;
  o = named("showfav"); if (o) showFav = o.getvalueof() === 1;
  state = KickBreeder.create(new Date().getTime() % 100000, { shuffle: shuffleOn, locks: locks });
  heard = "A";
  render();
}

// Hear a slot from the panel: it becomes the kick other notes play, and it plays now.
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
  state = KickBreeder.choose(state, index);
  heard = "A";
  render();
}
function choose_a(v) { choose(0, v); }
function choose_b(v) { choose(1, v); }
function choose_c(v) { choose(2, v); }

// Save the kick last heard into saved slot n (1 to 4), and into the set.
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

function back(v) { if (pressed(v) && state) { state = KickBreeder.back(state); heard = "A"; render(); } }
function reset(v) {
  if (!pressed(v) || !state) return;
  var i, o;
  for (i = 0; i < KickBreeder.PARAMS.length; i++) {
    o = named("lock_" + i);
    if (o) o.message("set", 0);
  }
  locks = {};
  state = KickBreeder.reset(state);
  heard = "A";
  render();
  show("status", "Reset: round 1, variation " + state.step.toFixed(2) + ", no locks. Saved kicks are kept.");
}
function breed_from_heard(v) {
  if (!pressed(v) || !state) return;
  state = KickBreeder.breedFrom(state, valuesOf(heard));
  heard = "A";
  render();
}

// From the panel: Variation slider, locks and toggles.
function variation(value) { if (state) { state = KickBreeder.setStep(state, value); render(); } }
function lock(index, value) {
  var id = KickBreeder.PARAMS[index].id;
  locks[id] = value === 1;
  if (state) state = KickBreeder.setLock(state, id, locks[id]);
}
function match(v) { matchOn = v === 1; render(); }
function shuffle(v) { shuffleOn = v === 1; if (state) state = KickBreeder.setShuffle(state, shuffleOn); }
function showfav(v) { showFav = v === 1; render(); }
