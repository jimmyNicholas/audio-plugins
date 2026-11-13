// Unit tests for the shared breeder maths (shared/breeder/dsp.js), run with node --test.
// dsp.js is a plain ES5 script for Max, so it is loaded here with vm.
const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

const context = {};
vm.runInNewContext(fs.readFileSync(path.join(__dirname, "../dsp.js"), "utf8") + "\nthis.D = BreederDsp;", context);
const D = context.D;
const SR = 22050;

const sine = (hz, f) => {
  const out = [];
  for (let i = 0; i < SR / 2; i++) out.push(f(Math.sin(2 * Math.PI * hz * i / SR)));
  return out;
};
const rms = (data) => Math.sqrt(data.slice(SR / 10).reduce((a, x) => a + x * x, 0) / (data.length - SR / 10));
const db = (x) => 20 * Math.log10(x);

test("weighting lifts 2 to 5 kHz by about 4 dB and leaves the lows alone", () => {
  const plain = rms(sine(200, (x) => x));
  const low = rms(sine(200, D.weighting(SR)));
  const high = rms(sine(4000, D.weighting(SR)));
  assert.ok(Math.abs(db(low / plain)) < 1, `200 Hz: ${db(low / plain).toFixed(2)} dB`);
  assert.ok(db(high / plain) > 3 && db(high / plain) < 4.5, `4 kHz: ${db(high / plain).toFixed(2)} dB`);
});

test("the filter's band-pass peaks at its frequency, and resonance narrows it", () => {
  const through = (hz, res) => {
    const f = D.svf(2000, res, SR);
    return rms(sine(hz, (x) => { f(x); return f.band; }));
  };
  assert.ok(through(2000, 0.5) > through(500, 0.5) && through(2000, 0.5) > through(8000, 0.5));
  const narrow = through(500, 0.9) / through(2000, 0.9);
  const wide = through(500, 0) / through(2000, 0);
  assert.ok(narrow < wide);
});

test("the filter stays stable up to its highest frequency", () => {
  const f = D.svf(20000, 1, SR);
  const noise = D.noise(1);
  let peak = 0;
  for (let i = 0; i < SR; i++) { f(noise()); peak = Math.max(peak, Math.abs(f.band), Math.abs(f.high)); }
  assert.ok(Number.isFinite(peak) && peak < 100);
});

test("matchGain hits the target, or stops at the peak limit", () => {
  assert.equal(D.matchGain({ rms: 0.1, peak: 0.3 }, 0.2, 0.95), 2);
  assert.ok(Math.abs(D.matchGain({ rms: 0.1, peak: 0.6 }, 0.2, 0.95) - 0.95 / 0.6) < 1e-12);
});

test("envelope helpers: decay ends silent, clicks fall, sweeps land on the end pitch", () => {
  const d = D.decayPoints(0.8, 200, 0);
  assert.equal(d[2], 0.8);
  assert.equal(d[d.length - 2], 0);
  const c = D.clickPoints(1);
  assert.ok(c[2] < c[0] && c[c.length - 2] === 0);
  const s = D.sweepPoints(400, 100, 50, 3);
  assert.equal(s[0], 400);
  assert.ok(Math.abs(s[s.length - 2] - 100) < 1e-9);
  assert.ok(Math.abs(D.valueAt(s, 1000) - 100) < 1e-9);
});
