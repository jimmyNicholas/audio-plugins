// Unit tests for the Kick Breeder spike engine (kick-breeder.js), run with node --test.
const test = require("node:test");
const assert = require("node:assert/strict");
const KB = require("../kick-breeder.js");

const favouriteIndex = (s) => s.candidates.findIndex((c) => c.favourite);
const newIndex = (s) => s.candidates.findIndex((c) => !c.favourite);
const inRange = (values) => values.every((v) => v >= 0 && v <= 1);

test("round 1 offers two kicks and no favourite", () => {
  const s = KB.create(1);
  assert.equal(s.round, 1);
  assert.equal(s.candidates.length, 2);
  assert.equal(KB.favourite(s), null);
  assert.ok(s.candidates.every((c) => c.values.length === KB.PARAMS.length && inRange(c.values)));
});

test("choosing keeps the pick as the favourite and adds two variations", () => {
  const s = KB.create(2);
  const picked = s.candidates[1].values;
  const next = KB.choose(s, 1);
  assert.equal(next.round, 2);
  assert.equal(next.candidates.length, 3);
  assert.equal(next.candidates.filter((c) => c.favourite).length, 1);
  assert.deepEqual(KB.favourite(next).values, picked);
  assert.ok(next.candidates.every((c) => inRange(c.values)));
  assert.equal(s.round, 1, "the old state is not changed");
});

test("choosing the favourite again shrinks the variations; a new one keeps the size", () => {
  let s = KB.choose(KB.create(3), 0);
  assert.equal(s.step, KB.START_STEP);
  s = KB.choose(s, newIndex(s));
  assert.equal(s.step, KB.START_STEP);
  s = KB.choose(s, favouriteIndex(s));
  assert.ok(Math.abs(s.step - KB.START_STEP * 0.6) < 1e-12);
  for (let i = 0; i < 20; i++) s = KB.choose(s, favouriteIndex(s));
  assert.equal(s.step, KB.MIN_STEP);
});

test("a locked setting never changes in variations", () => {
  let s = KB.setLock(KB.create(4), "decay", true);
  s = KB.choose(s, 0);
  const decay = KB.favourite(s).values[4];
  for (let i = 0; i < 10; i++) {
    assert.ok(s.candidates.every((c) => c.values[4] === decay));
    s = KB.choose(s, newIndex(s));
  }
});

test("Back undoes a choice; Reset starts again with no locks and the starting size", () => {
  const one = KB.choose(KB.create(5), 0);
  const two = KB.choose(one, 2);
  assert.deepEqual(KB.back(two).candidates, one.candidates);
  assert.equal(KB.back(two).round, one.round);
  assert.equal(KB.back(KB.create(5)).round, 1, "nothing to go back to");
  const fresh = KB.reset(KB.setStep(KB.setLock(two, "pitch", true), 0.05));
  assert.equal(fresh.round, 1);
  assert.equal(fresh.candidates.length, 2);
  assert.equal(fresh.step, KB.START_STEP);
  assert.deepEqual(fresh.locks, {});
  assert.equal(fresh.past.length, 0);
});

test("the Variation slider sets the size and breeds the variations again", () => {
  const start = KB.create(6);
  const early = KB.setStep(start, 0.1);
  assert.equal(early.step, 0.1);
  assert.deepEqual(early.candidates, start.candidates, "no favourite yet, so round 1 kicks stay");
  const s = KB.choose(start, 0);
  const wide = KB.setStep(s, 0.45);
  assert.equal(wide.step, 0.45);
  assert.deepEqual(KB.favourite(wide).values, KB.favourite(s).values);
  assert.notDeepEqual(wide.candidates, s.candidates);
  assert.equal(wide.past.length, s.past.length, "dragging the slider does not fill Back");
  assert.equal(KB.setStep(s, 9).step, KB.MAX_STEP);
  assert.equal(KB.setStep(s, 0).step, KB.MIN_STEP);
});

test("the same seed gives the same search", () => {
  const run = () => KB.choose(KB.choose(KB.create(7), 1), 2);
  assert.deepEqual(run(), run());
});

test("with Shuffle off the favourite is always slot A", () => {
  let s = KB.setShuffle(KB.create(8), false);
  for (let i = 0; i < 5; i++) {
    s = KB.choose(s, i % 3 === 0 ? 0 : 1);
    assert.equal(favouriteIndex(s), 0);
  }
});

test("Breed from kept starts a search around the kept kick", () => {
  const kept = KB.PARAMS.map(() => 0.5);
  const s = KB.breedFrom(KB.create(9), kept);
  assert.equal(s.candidates.length, 3);
  assert.deepEqual(KB.favourite(s).values, kept);
});

test("real values span each setting's range", () => {
  for (const p of KB.PARAMS) {
    assert.ok(Math.abs(KB.toReal(p, 0) - p.min) < 1e-9, p.id);
    assert.ok(Math.abs(KB.toReal(p, 1) - p.max) < 1e-6, p.id);
  }
});

test("compression lifts the tail against the hit, and 0 leaves it alone", () => {
  for (const level of [1, 0.5, 0.1, 0.01]) assert.ok(Math.abs(KB.compress(level, 0) - level) < 1e-12);
  assert.equal(KB.compress(1, 1), 1, "the peak stays at full scale");
  assert.ok(KB.compress(0.1, 1) > 0.1);
  assert.ok(KB.compress(0.1, 1) > KB.compress(0.1, 0.5));
  assert.equal(KB.compress(0, 1), 0);
});

test("the noise layer follows Noise level and Noise decay", () => {
  const values = KB.PARAMS.map(() => 0.5);
  const quiet = values.slice();
  quiet[7] = 0;
  const v = KB.voice(values, false);
  const p = KB.realValues(values);
  assert.ok(Math.abs(v.noise[2] - p.noiseLevel * 0.6) < 1e-12);
  const time = (pairs) => pairs.filter((_, i) => i % 2 === 1).reduce((a, b) => a + b, 0);
  assert.ok(Math.abs(time(v.noise) - (p.noiseDecay + 6.5)) < 1e-6);
  assert.ok(KB.voice(quiet, false).noise.every((x, i) => i % 2 === 1 || x === 0));
});

test("the settings list matches the patch's unpack order", () => {
  const values = KB.PARAMS.map(() => 0.5);
  const p = KB.realValues(values);
  const s = KB.voice(values, false).statics;
  assert.equal(s.length, 6);
  assert.equal(s[0], p.clickTone);
  assert.equal(s[1], p.noiseTone);
  assert.equal(s[4], p.tone);
  assert.equal(s[5], 0.5, "raw gain when not matching");
});

test("the voice sweeps from the start pitch down to Pitch over Sweep time", () => {
  const values = KB.PARAMS.map(() => 0.5);
  const p = KB.realValues(values);
  const v = KB.voice(values, false);
  assert.ok(Math.abs(v.freq[0] - p.pitch * Math.pow(2, p.sweep / 12)) < 1e-6);
  assert.equal(v.freq[1], 0, "jumps to the start");
  assert.ok(Math.abs(v.freq[v.freq.length - 2] - p.pitch) < 1e-6);
  const time = (pairs) => pairs.filter((_, i) => i % 2 === 1).reduce((a, b) => a + b, 0);
  assert.ok(Math.abs(time(v.freq) - p.sweepTime) < 1e-6);
  assert.ok(Math.abs(time(v.amp) - (p.decay + 6.5)) < 1e-6);
  assert.equal(v.amp[v.amp.length - 2], 0, "ends silent");
  assert.ok([v.freq, v.amp, v.click, v.noise].every((e) => e.length / 2 <= 64), "fits in line~");
});

test("matching brings different kicks to about the same loudness", () => {
  let s = KB.create(10);
  const levels = [];
  for (let i = 0; i < 6; i++) {
    for (const c of s.candidates) {
      const v = KB.voice(c.values, true);
      const l = KB.loudness(KB.realValues(c.values));
      if (l.peak * v.statics[5] < 0.949) levels.push(l.rms * v.statics[5]);
    }
    s = KB.reset(s);
  }
  assert.ok(levels.length >= 6);
  for (const level of levels) assert.ok(Math.abs(level - 0.25) < 1e-9);
});

test("the matched gain never pushes the simulated peak past the limit", () => {
  let s = KB.create(11);
  for (let i = 0; i < 10; i++) {
    for (const c of s.candidates) {
      const l = KB.loudness(KB.realValues(c.values));
      assert.ok(l.peak * KB.voice(c.values, true).statics[5] <= 0.95 + 1e-9);
    }
    s = KB.reset(s);
  }
});

test("describe names the main settings", () => {
  const text = KB.describe(KB.PARAMS.map(() => 0.5));
  assert.match(text, /Hz, sweep \d+ st in \d+ ms, decay \d+ ms, click [\d.]+, noise [\d.]+, comp [\d.]+, sat [\d.]+/);
});
