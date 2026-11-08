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

test("Back undoes a choice, and Fresh starts again with the same locks", () => {
  const one = KB.choose(KB.create(5), 0);
  const two = KB.choose(one, 2);
  assert.deepEqual(KB.back(two).candidates, one.candidates);
  assert.equal(KB.back(two).round, one.round);
  assert.equal(KB.back(KB.create(5)).round, 1, "nothing to go back to");
  const locked = KB.setLock(two, "pitch", true);
  const fresh = KB.fresh(locked);
  assert.equal(fresh.round, 1);
  assert.equal(fresh.candidates.length, 2);
  assert.equal(fresh.locks.pitch, true);
});

test("Wider and Closer resize the variations around the favourite", () => {
  const start = KB.create(6);
  assert.equal(KB.wider(start), start, "no favourite yet");
  const s = KB.choose(start, 0);
  const wide = KB.wider(s);
  assert.ok(wide.step > s.step);
  assert.deepEqual(KB.favourite(wide).values, KB.favourite(s).values);
  assert.ok(KB.closer(s).step < s.step);
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

test("real values round-trip", () => {
  for (const p of KB.PARAMS) {
    for (const v of [0, 0.25, 0.5, 1]) assert.ok(Math.abs(KB.fromReal(p, KB.toReal(p, v)) - v) < 1e-9);
  }
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
  assert.ok(v.freq.length / 2 <= 64 && v.amp.length / 2 <= 64 && v.click.length / 2 <= 64, "fits in line~");
});

test("matching brings different kicks to about the same loudness", () => {
  let s = KB.create(10);
  const levels = [];
  for (let i = 0; i < 6; i++) {
    for (const c of s.candidates) {
      const v = KB.voice(c.values, true);
      const l = KB.loudness(KB.realValues(c.values));
      if (l.peak * v.gain < 0.949) levels.push(l.rms * v.gain);
    }
    s = KB.fresh(s);
  }
  assert.ok(levels.length >= 6);
  for (const level of levels) assert.ok(Math.abs(level - 0.25) < 1e-9);
});

test("the matched gain never pushes the simulated peak past the limit", () => {
  let s = KB.create(11);
  for (let i = 0; i < 10; i++) {
    for (const c of s.candidates) {
      const l = KB.loudness(KB.realValues(c.values));
      assert.ok(l.peak * KB.voice(c.values, true).gain <= 0.95 + 1e-9);
    }
    s = KB.fresh(s);
  }
});

test("describe names the main settings", () => {
  const text = KB.describe(KB.PARAMS.map(() => 0.5));
  assert.match(text, /Hz, sweep \d+ st in \d+ ms, decay \d+ ms/);
});
