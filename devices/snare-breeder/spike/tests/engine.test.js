// Unit tests for the Snare Breeder spike's voice (snare-voice.js, joined into snare-breeder.js),
// run with node --test. The breeding itself is shared and tested through the kick spike.
const test = require("node:test");
const assert = require("node:assert/strict");
const SB = require("../snare-breeder.js");

const index = (id) => SB.PARAMS.findIndex((p) => p.id === id);
const mid = () => SB.PARAMS.map(() => 0.5);
const withValue = (values, id, v) => { const out = values.slice(); out[index(id)] = v; return out; };
const time = (pairs) => pairs.filter((_, i) => i % 2 === 1).reduce((a, b) => a + b, 0);
const peaks = (pairs, level) => pairs.filter((x, i) => i % 2 === 0 && Math.abs(x - level) < 1e-12).length;

test("breeding works on the snare's sixteen settings", () => {
  assert.equal(SB.PARAMS.length, 16);
  let s = SB.create(1);
  s = SB.choose(s, 0);
  assert.equal(s.candidates.length, 3);
  assert.ok(s.candidates.every((c) => c.values.length === 16 && c.values.every((v) => v >= 0 && v <= 1)));
});

test("half of all snares have one burst; the rest have two to four", () => {
  assert.equal(SB.burstCount(0), 1);
  assert.equal(SB.burstCount(0.49), 1);
  assert.equal(SB.burstCount(0.5), 2);
  assert.equal(SB.burstCount(0.7), 3);
  assert.equal(SB.burstCount(0.9), 4);
  assert.equal(SB.burstCount(1), 4);
});

test("the wires envelope has one peak per burst, Spread apart, then the tail", () => {
  for (const [v, n] of [[0, 1], [0.55, 2], [0.7, 3], [1, 4]]) {
    const values = withValue(mid(), "bursts", v);
    const p = SB.realValues(values);
    const level = Math.sin(p.balance * Math.PI / 2);
    const wires = SB.voice(values, false).wires;
    assert.equal(peaks(wires, level), n, `${n} bursts`);
    assert.ok(Math.abs(time(wires) - ((n - 1) * p.spread + p.wiresDecay + 6.5)) < 1e-6);
    assert.equal(wires[wires.length - 2], 0, "ends silent");
    assert.ok(wires.length / 2 <= 64, "fits in line~");
  }
});

test("Balance moves the level from the body to the wires", () => {
  const allBody = SB.voice(withValue(mid(), "balance", 0), false);
  const allWires = SB.voice(withValue(mid(), "balance", 1), false);
  assert.ok(Math.abs(allBody.amp[2] - 1) < 1e-12);
  assert.ok(allBody.wires.every((x, i) => i % 2 === 1 || Math.abs(x) < 1e-12), "no wires");
  assert.ok(allWires.amp.every((x, i) => i % 2 === 1 || Math.abs(x) < 1e-12), "no body");
});

test("the body sweeps from the start pitch down to Pitch over Sweep time", () => {
  const values = mid();
  const p = SB.realValues(values);
  const v = SB.voice(values, false);
  assert.ok(Math.abs(v.freq[0] - p.pitch * Math.pow(2, p.sweep / 12)) < 1e-6);
  assert.ok(Math.abs(v.freq[v.freq.length - 2] - p.pitch) < 1e-6);
  assert.ok(Math.abs(time(v.freq) - p.sweepTime) < 1e-6);
  assert.ok(Math.abs(time(v.amp) - (p.bodyDecay + 6.5)) < 1e-6);
});

test("the settings list matches the patch's unpack order", () => {
  const values = mid();
  const p = SB.realValues(values);
  const s = SB.voice(values, false).statics;
  assert.deepEqual(s.slice(0, 3), [p.ratio, p.ring, p.wiresTone]);
  assert.ok(Math.abs(s[3] - 0.8 * (1 - p.width)) < 1e-12, "resonance from Width");
  assert.equal(s[6], p.tone);
  assert.equal(s[7], 0.5, "raw gain when not matching");
});

test("matching brings different snares to about the same loudness", () => {
  let s = SB.create(10);
  const levels = [];
  for (let i = 0; i < 8; i++) {
    for (const c of s.candidates) {
      const gain = SB.voice(c.values, true).statics[7];
      const l = SB.loudness(SB.realValues(c.values));
      assert.ok(Number.isFinite(gain) && gain > 0);
      assert.ok(l.peak * gain <= 0.95 + 1e-9, "never past the peak limit");
      if (l.peak * gain < 0.949) levels.push(l.rms * gain);
    }
    s = SB.reset(s);
  }
  assert.ok(levels.length >= 6, `only ${levels.length} snares were not peak-limited`);
  for (const level of levels) assert.ok(Math.abs(level - 0.2) < 1e-9);
});

test("the extremes of every setting give a finite, audible snare", () => {
  for (const v of [0, 1]) {
    const values = SB.PARAMS.map(() => v);
    const l = SB.loudness(SB.realValues(values));
    assert.ok(Number.isFinite(l.rms) && l.rms > 0 && Number.isFinite(l.peak), `all ${v}`);
  }
});

test("describe and the saved label name the main settings", () => {
  assert.match(SB.describe(mid()), /Hz, ring [\d.]+ at [\d.]+x, body \d+ ms, wires \d+ ms at [\d.]+ k?Hz, balance [\d.]+, 2 bursts, snap/);
  assert.match(SB.label(withValue(mid(), "bursts", 0)), / snare$/);
  assert.match(SB.label(withValue(mid(), "bursts", 1)), / clap$/);
});
