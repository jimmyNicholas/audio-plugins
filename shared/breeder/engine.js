// Breeder devices: the breeding engine, the same for every drum. ES5 (Max 8's js object).
// Joined into each device's js by its build; see shared/breeder/README.md.
//
// makeBreeder(voice) takes a drum voice (its PARAMS, and functions such as voice and describe)
// and returns one object with the breeding functions and the voice's functions together. Pure:
// every function returns a new state and leaves the old one alone.

function makeBreeder(drum) {
  var PARAMS = drum.PARAMS;
  var START_STEP = 0.25, MIN_STEP = 0.02, MAX_STEP = 0.5, SHRINK = 0.6;
  var MODULUS = BreederDsp.MODULUS;

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
  // Start again from round 1: new drums, the starting variation size, and no locks.
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

  var engine = {
    PARAMS: PARAMS, START_STEP: START_STEP, MIN_STEP: MIN_STEP, MAX_STEP: MAX_STEP,
    create: create, choose: choose, setStep: setStep, back: back, reset: reset,
    setLock: setLock, setShuffle: setShuffle, breedFrom: breedFrom, favourite: favourite,
    toReal: BreederDsp.toReal,
    realValues: function (values) { return BreederDsp.realValues(PARAMS, values); },
    compress: BreederDsp.compress, valueAt: BreederDsp.valueAt
  };
  var key;
  for (key in drum) if (drum.hasOwnProperty(key) && !engine.hasOwnProperty(key)) engine[key] = drum[key];
  return engine;
}
