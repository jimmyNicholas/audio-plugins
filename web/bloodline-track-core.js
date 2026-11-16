/* PROTOTYPE, throwaway. Bloodline Track core (issue #51, phase 2): the Track's state, the actions on it,
   and its offline render to stems and a mix. No DOM, so it runs in Node for tests and in the page for sound.
   - Drums: Bloodline Drums' voice and zones, and the break writer, ported from the Breaks mobile prototype.
     The Notes per bar shaping is gone; each loop carries eight traits instead (Tight, Dynamic, Ghosts,
     Dense, Off beat, Repeat, Fills, Anchored), for all rows or one row. A kept pad's label sets its row's part.
   - Bass and synth: a plain-JS synth voice on the shared engine's settings, and note loops from the shared
     engine's eight traits. A bass can lean onto a drum lane's kick.
   - The groove is global: swing, accents and drift apply to every lane at render time.
   - Effects: the shared engine's chains, run offline through an injected function (opts.fx); without one
     (in Node) chains pass audio through untouched.
   - Render: each lane to a stereo stem; level, pan, mute and solo after; the master sums the stems, runs
     its effects, its level, then the -1 dB limiter. Loops are seamless: tails wrap round to the start. */
(function (global) {
'use strict';
const BL = global.BL;
if (!BL) throw new Error('Load bloodline-engine.js before bloodline-track-core.js');

const SR = 44100;
const clamp = (x, a, b) => (x < a ? a : x > b ? b : x);
const lerp = (a, b, t) => a + (b - a) * t;
const fold = v => { v = Math.abs(v); if (v > 1) v = 2 - v; return clamp(v, 0, 1); };
const dbg = db => db === -Infinity ? 0 : Math.pow(10, db / 20);
const clone = o => JSON.parse(JSON.stringify(o));
function mulberry32(a) { return function () { a |= 0; a = a + 0x6D2B79F5 | 0; let t = Math.imul(a ^ a >>> 15, 1 | a); t = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t; return ((t ^ t >>> 14) >>> 0) / 4294967296; }; }
let gen = Math.random;
const R = () => gen();
function seed(n) { gen = mulberry32(n >>> 0); }
function withSeed(n, fn) { const old = gen; gen = mulberry32(n >>> 0); try { return fn(); } finally { gen = old; } }
function gaussOf(r) { let u = 0; while (!u) u = r(); return Math.sqrt(-2 * Math.log(u)) * Math.cos(2 * Math.PI * r()); }
const gauss = () => gaussOf(R);
function poisson(l) { const E = Math.exp(-l); let k = 0, p = 1; do { k++; p *= R(); } while (p > E); return k - 1; }
const newSeed = () => Math.floor(R() * 2147483646) + 1;
const L4 = 'ABCD';
const KEYS = ['C', 'C#', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B'];
const SCALES = ['Minor', 'Major', 'Dorian', 'Phrygian', 'Minor pent', 'Major pent', 'Blues'];

// =====================================================================================
// Drum voice (ported from plugins/bloodline-drums/core via the Breaks mobile prototype)
// =====================================================================================
const SET = [ // name, group, unit, min, max, log, bred
  ['Pitch', 0, 'Hz', 20, 2000, 1, 1], ['Sweep', 0, 'st', 0, 60, 0, 1], ['Sweep time', 0, 's', 0.002, 0.3, 1, 1], ['Attack', 0, 's', 0, 0.05, 0, 1], ['Decay', 0, 's', 0.005, 4, 1, 1],
  ['Body / Noise', 1, '%', 0, 1, 0, 1], ['Noise tone', 1, 'Hz', 100, 16000, 1, 1], ['Noise decay', 1, 's', 0.005, 4, 1, 1], ['Metal', 1, '%', 0, 1, 0, 1],
  ['Click', 2, '%', 0, 1, 0, 1], ['Click tone', 2, 'Hz', 500, 12000, 1, 1],
  ['Drive', 3, '%', 0, 1, 0, 1], ['Bits', 3, 'bits', 16, 2, 0, 1], ['Rate', 3, 'Hz', 48000, 1000, 1, 1],
  ['High-pass', 4, 'Hz', 20, 8000, 1, 1], ['Low-pass', 4, 'Hz', 200, 20000, 1, 1], ['Resonance', 4, '%', 0, 0.95, 0, 1], ['Pan', 4, 'pan', -1, 1, 0, 1], ['Level', 4, 'dB', -24, 0, 0, 0],
];
const GROUPS = ['Body', 'Noise', 'Click', 'Grit', 'Out'];
const PAN = 17, LEVEL = 18;
const ZONES = {
  Kick: [[40, 90], [3, 24], [0.015, 0.08], [0, 0.003], [0.15, 0.6], [0, 0.25], [200, 2000], [0.01, 0.08], [0, 0.05], [0.1, 0.8], [1000, 5000]],
  Snare: [[150, 280], [0, 8], [0.01, 0.05], [0, 0.002], [0.08, 0.35], [0.4, 0.8], [1500, 6000], [0.1, 0.4], [0, 0.2], [0, 0.6], [2000, 8000]],
  Hats: [[100, 400], [0, 2], [0.002, 0.02], [0, 0.001], [0.03, 0.8], [0.85, 1], [5000, 16000], [0.03, 0.8], [0.4, 1], [0, 0.4], [4000, 12000]],
  Perc: [[200, 900], [0, 6], [0.005, 0.04], [0, 0.002], [0.02, 0.2], [0.2, 0.6], [1500, 8000], [0.02, 0.15], [0, 0.3], [0.2, 0.8], [1500, 6000]],
};
const LABELS = ['Kick', 'Snare', 'Hats', 'Perc', 'None'];
const TUNE = { start: 0.25, min: 0.02, max: 0.5, narrow: 0.6, widen: 1.6, zoneShare: 0.8, pull: 0.5 };
function real(i, n) { const s = SET[i]; n = clamp(n, 0, 1); return s[5] ? s[3] * Math.pow(s[4] / s[3], n) : s[3] + (s[4] - s[3]) * n; }
function norm(i, x) { const s = SET[i]; x = clamp(x, Math.min(s[3], s[4]), Math.max(s[3], s[4])); return s[5] ? Math.log(x / s[3]) / Math.log(s[4] / s[3]) : (x - s[3]) / (s[4] - s[3]); }
function zoneRange(zone, i) { if (!ZONES[zone] || i > 10) return [0, 1]; const z = ZONES[zone][i], a = norm(i, z[0]), b = norm(i, z[1]); return [Math.min(a, b), Math.max(a, b)]; }
function fmt(i, n) {
  const v = real(i, n), u = SET[i][2];
  if (u === 'Hz') return v >= 1000 ? (v / 1000).toFixed(1) + ' kHz' : Math.round(v) + ' Hz';
  if (u === 's') return v < 1 ? (v * 1000).toFixed(v < 0.01 ? 1 : 0) + ' ms' : v.toFixed(2) + ' s';
  if (u === 'st') return v.toFixed(1) + ' st';
  if (u === 'bits') return v.toFixed(1) + ' bits';
  if (u === 'dB') return v.toFixed(1) + ' dB';
  if (u === 'pan') return Math.abs(v) < 0.02 ? 'C' : (v < 0 ? 'L' : 'R') + Math.round(Math.abs(v) * 100);
  return Math.round(v * 100) + '%';
}
const drumOps = {
  fresh(zone) { const v = SET.map((s, i) => { const r = zone !== 'None' && R() < TUNE.zoneShare ? zoneRange(zone, i) : [0, 1]; return lerp(r[0], r[1], R()); }); v[LEVEL] = 1; if (zone === 'Kick' || zone === 'Snare') v[PAN] = 0.5; else v[PAN] = 0.35 + R() * 0.3; return v; },
  mutate(from, spread, locks, zone) { return from.map((x, i) => { if (!SET[i][6] || locks[i]) return x; let y = fold(x + gauss() * spread); if (zone !== 'None') { const [lo, hi] = zoneRange(zone, i); if (y < lo) y += (lo - y) * TUNE.pull; else if (y > hi) y -= (y - hi) * TUNE.pull; } return y; }); },
  cross(a, b, locks) { const pick = GROUPS.map(() => R() < 0.5); return a.map((x, i) => locks[i] ? x : pick[SET[i][1]] ? x : b[i]); },
};
const K808 = [205.3, 304.4, 369.6, 522.7, 540, 800];
const coefFor = (sec, target) => Math.exp(Math.log(target) / Math.max(1, sec * SR));
const gFor = hz => Math.tan(Math.PI * clamp(hz, 10, 0.45 * SR) / SR);
function drumVoice(vals, h, sd, maxN) {
  const r = vals.map((x, i) => real(i, x));
  const dm = h.dm || 1; r[4] *= dm; r[7] *= dm; r[6] *= h.br || 1; r[10] *= h.br || 1;
  const vel = clamp(h.vel, 0, 1), harder = 0.15 * vel * vel;
  const pitch = r[0], sweepOct = r[1] / 12, sweepCoef = coefFor(r[2], 0.001), ampCoef = coefFor(r[4], 0.001), noiseCoef = coefFor(r[7], 0.001);
  const noiseMix = r[5], metalAmt = r[8], click = Math.min(1, r[9] + harder);
  const drive = Math.min(1, r[11] + harder), driveGain = 1 + 30 * drive * drive, driveNorm = 1 / Math.tanh(driveGain), hard = clamp((drive - 0.8) / 0.2, 0, 1);
  const steps = Math.pow(2, r[12] - 1), holdInc = Math.min(1, r[13] / SR), level = 0.5 * vel * dbg(clamp(h.levelDb, -60, 6));
  const svfK = 1.4, fK = 2 - 2 * r[16], co = (g, k) => { const a1 = 1 / (1 + g * (g + k)); return [a1, g * a1, g * g * a1]; };
  const [na1, na2, na3] = co(gFor(r[6]), svfK), [ca1, ca2, ca3] = co(gFor(r[10]), 1), [ha1, ha2, ha3] = co(gFor(r[14]), fK), [la1, la2, la3] = co(gFor(r[15]), fK);
  const sqInc = K808.map(f => f * pitch / 200 / SR), sqPh = [0, 0, 0, 0, 0, 0];
  const attackStep = 1 / Math.max(1, Math.max(0.001, r[3]) * SR), nAttackStep = 1 / Math.max(1, 0.0005 * SR), clickCoef = coefFor(0.004, 0.001);
  let phase = 0, sweepEnv = 1, amp = 1, noiseAmp = 1, attack = 0, nAttack = 0, clickEnv = 1, held = 0, holdPh = 1, noise = (sd >>> 0) || 0x9e3779b9;
  let n1 = 0, n2 = 0, m1 = 0, m2 = 0, c1 = 0, c2 = 0, h1 = 0, h2 = 0, l1 = 0, l2 = 0;
  const N = Math.min(maxN, 5 * SR), out = new Float32Array(N); let n = 0;
  for (; n < N; n++) {
    phase += Math.min(pitch * Math.pow(2, sweepOct * sweepEnv), 0.45 * SR) / SR; phase -= Math.floor(phase);
    const body = Math.sin(2 * Math.PI * phase) * amp * attack;
    sweepEnv *= sweepCoef; amp *= ampCoef; attack = Math.min(1, attack + attackStep);
    noise ^= noise << 13; noise >>>= 0; noise ^= noise >>> 17; noise ^= noise << 5; noise >>>= 0;
    const white = noise / 4294967295 * 2 - 1;
    let v3 = white - n2, v1 = na1 * n1 + na2 * v3, v2 = n2 + na2 * n1 + na3 * v3; n1 = 2 * v1 - n1; n2 = 2 * v2 - n2;
    const nz = v1 * svfK * 2.5;
    let sq = 0; for (let k = 0; k < 6; k++) { sqPh[k] += sqInc[k]; sqPh[k] -= Math.floor(sqPh[k]); sq += sqPh[k] < 0.5 ? 1 : -1; } sq /= 6;
    v3 = sq - m2; v1 = na1 * m1 + na2 * v3; v2 = m2 + na2 * m1 + na3 * v3; m1 = 2 * v1 - m1; m2 = 2 * v2 - m2;
    const metal = (sq - svfK * v1 - v2) * 1.5;
    const noisePart = ((1 - metalAmt) * nz + metalAmt * metal) * noiseAmp * nAttack;
    noiseAmp *= noiseCoef; nAttack = Math.min(1, nAttack + nAttackStep);
    let s = (1 - noiseMix) * body + noiseMix * noisePart;
    v3 = white - c2; v1 = ca1 * c1 + ca2 * v3; v2 = c2 + ca2 * c1 + ca3 * v3; c1 = 2 * v1 - c1; c2 = 2 * v2 - c2;
    s += click * clickEnv * v1 * 2; clickEnv *= clickCoef;
    const soft = Math.tanh(s * driveGain) * driveNorm, hd = clamp(s * driveGain, -1, 1); s = soft + (hd - soft) * hard;
    s = Math.round(s * steps) / steps;
    holdPh += holdInc; if (holdPh >= 1) { holdPh -= 1; held = s; } s = held;
    v3 = s - h2; v1 = ha1 * h1 + ha2 * v3; v2 = h2 + ha2 * h1 + ha3 * v3; h1 = 2 * v1 - h1; h2 = 2 * v2 - h2; s = s - fK * v1 - v2;
    v3 = s - l2; v1 = la1 * l1 + la2 * v3; v2 = l2 + la2 * l1 + la3 * v3; l1 = 2 * v1 - l1; l2 = 2 * v2 - l2;
    s = v2 * level; if (!isFinite(s)) { s = 0; h1 = h2 = l1 = l2 = 0; }
    const mag = Math.abs(s); if (mag > 0.7) s = Math.sign(s) * (0.7 + 0.3 * Math.tanh((mag - 0.7) / 0.3));
    out[n] = s;
    if (amp < 1e-4 && noiseAmp < 1e-4 && clickEnv < 1e-4 && n > 64) { n++; break; }
  }
  const res = out.subarray(0, n);
  if (n >= maxN) { const f = Math.min(n, Math.round(0.002 * SR)); for (let i = 0; i < f; i++) res[n - 1 - i] *= i / f; } // cut by the same pad's next hit
  return res;
}

// =====================================================================================
// Synth voice: the shared engine's 17 settings, rendered offline in plain JS
// =====================================================================================
const SY = BL.SYNTH;
const PZONES = {
  bass: {
    Sub: { 'Shape A': [0, 0.2], 'Mix B': [0, 0.15], Sub: [0.6, 1], Detune: [0, 6], Cutoff: [120, 700], Resonance: [0.5, 2], 'Env amount': [0, 1], 'Filter decay': [50, 300], Attack: [1, 8], Decay: [200, 900], Sustain: [0.6, 1], Release: [30, 160], Vibrato: [0, 0.01], Drive: [0, 0.3] },
    Pluck: { 'Shape A': [0.3, 1], Sub: [0.2, 0.6], Cutoff: [200, 1500], Resonance: [1, 6], 'Env amount': [1.5, 4], 'Filter decay': [40, 250], Attack: [1, 5], Decay: [80, 400], Sustain: [0, 0.25], Release: [40, 200], Vibrato: [0, 0.02], Drive: [0, 0.5] },
    Round: BL.SYNTH_ZONES.Bass,
    Growl: { 'Shape A': [0.5, 1], 'Shape B': [0.5, 1], 'Mix B': [0.3, 0.8], Detune: [5, 25], Sub: [0.2, 0.6], Cutoff: [200, 1500], Resonance: [3, 12], 'Env amount': [1, 4], 'Filter decay': [60, 300], Attack: [1, 10], Sustain: [0.4, 0.9], Release: [30, 200], Drive: [0.4, 1] },
  },
  synth: { Pad: BL.SYNTH_ZONES.Pad, Pluck: BL.SYNTH_ZONES.Pluck, Lead: BL.SYNTH_ZONES.Lead, Keys: BL.SYNTH_ZONES.Keys },
};
const PRANGE = { bass: [28, 52], synth: [52, 84] };
const ZONE_NAMES = { bass: Object.keys(PZONES.bass), synth: Object.keys(PZONES.synth) };
function pRanges(kind, zone) { const Z = (PZONES[kind] || {})[zone] || {}; return SY.map(s => Z[s.name] ? [BL.toNorm(s, Z[s.name][0]), BL.toNorm(s, Z[s.name][1])] : [0, 1]); }
const voiceOps = {
  fresh(kind, zone) { const Z = pRanges(kind, zone); return Z.map(([lo, hi]) => R() < 0.9 ? lerp(lo, hi, R()) : R()); },
  mutate(v, spread, locks, kind, zone) { const Z = pRanges(kind, zone); return v.map((x, i) => { if (locks[i]) return x; let y = fold(x + gauss() * spread); const [lo, hi] = Z[i]; if (y < lo) y += (lo - y) * TUNE.pull; else if (y > hi) y -= (y - hi) * TUNE.pull; return y; }); },
  cross(a, b, locks) { const groups = [...new Set(SY.map(s => s.group))], pick = {}; groups.forEach(g => { pick[g] = R() < 0.5; }); return a.map((x, i) => locks[i] ? x : pick[SY[i].group] ? x : b[i]); },
};
function blep(t, dt) { if (t < dt) { t /= dt; return t + t - t * t - 1; } if (t > 1 - dt) { t = (t - 1) / dt; return t * t + t + t + 1; } return 0; }
function osc(w, ph, dt) {
  if (w === 0) return Math.sin(2 * Math.PI * ph);
  if (w === 1) return 1 - 4 * Math.abs(ph - 0.5);
  if (w === 2) return 2 * ph - 1 - blep(ph, dt);
  let s = ph < 0.5 ? 1 : -1; s += blep(ph, dt); s -= blep((ph + 0.5) % 1, dt); return s;
}
function synthVoice(vn, midi, dur, vel) {
  const v = i => BL.toReal(SY[i], vn[i]);
  const wa = Math.min(3, Math.floor(vn[0] * 4)), wb = Math.min(3, Math.floor(vn[1] * 4));
  const mixB = v(2), tuneB = Math.round(v(3)), det = v(4), sub = v(5), cut = v(6), q = v(7), env = v(8), fd = v(9) / 1000;
  const at = Math.max(0.001, v(10) / 1000), de = v(11) / 1000, su = v(12), re = v(13) / 1000, vib = v(14), lr = v(15), drv = v(16);
  const f0 = 440 * Math.pow(2, (midi - 69) / 12), hold = Math.max(dur, at), N = Math.min(Math.round((hold + re * 1.6 + 0.02) * SR), 10 * SR);
  const out = new Float32Array(N), k = 1 / q, dg = 1 + drv * 8, dn = 1 / Math.tanh(dg);
  const decC = Math.exp(-1 / Math.max(1, de / 4 * SR)), relC = Math.exp(-1 / Math.max(1, re / 5 * SR)), fC = Math.exp(-1 / Math.max(1, fd / 3 * SR));
  const fa = f0 * Math.pow(2, -det / 2400), fb = f0 * Math.pow(2, tuneB / 12 + det / 2400), gA = 1 - mixB * 0.7, gB = mixB, gS = sub * 0.9;
  let pa = 0, pb = 0, ps = 0, amp = 0, fenv = 1, s1 = 0, s2 = 0, released = false, rel0 = 0;
  const pk = vel * 0.25, holdN = Math.round(hold * SR), atN = Math.round(at * SR);
  let n = 0;
  for (; n < N; n++) {
    if (n < atN) amp = pk * n / atN;
    else if (n < holdN) amp = pk * su + (amp - pk * su) * decC;
    else { if (!released) { released = true; rel0 = amp; } amp *= relC; }
    const vb = vib ? Math.pow(2, vib * Math.sin(2 * Math.PI * lr * n / SR) / 12) : 1;
    const da = fa * vb / SR, db = fb * vb / SR, ds = f0 * 0.5 * vb / SR;
    let s = (gA > 0.01 ? osc(wa, pa, da) * gA : 0) + (gB > 0.01 ? osc(wb, pb, db) * gB : 0) + (gS > 0.01 ? Math.sin(2 * Math.PI * ps) * gS : 0);
    pa += da; pa -= Math.floor(pa); pb += db; pb -= Math.floor(pb); ps += ds; ps -= Math.floor(ps);
    const fc = Math.min(18000, cut * Math.pow(2, env * fenv)); fenv *= fC;
    const g = Math.tan(Math.PI * Math.min(fc, 0.45 * SR) / SR), a1 = 1 / (1 + g * (g + k)), a2 = g * a1, a3 = g * a2;
    const v3 = s - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3; s1 = 2 * v1 - s1; s2 = 2 * v2 - s2;
    s = v2; if (drv > 0.01) s = Math.tanh(s * dg) * dn;
    s *= amp; if (!isFinite(s)) { s = 0; s1 = s2 = 0; }
    out[n] = s;
    if (released && amp < pk * 1e-4 && n > holdN + 64) { n++; break; }
  }
  const res = out.subarray(0, n), f = Math.min(n, 64); for (let i = 0; i < f; i++) res[n - 1 - i] *= i / f;
  return res;
}
const PHRASES = { 'Root and fifth': [[0, 0, 1], [1, 4, 1], [2, 0, 1], [3, 4, 1]], 'Scale run': [0, 1, 2, 3, 4, 5, 6, 7].map((d, i) => [i * 0.5, d, 0.5]), Octaves: [[0, 0, 0.5], [0.5, 7, 0.5], [1, 0, 0.5], [1.5, 7, 0.5], [2, 0, 0.5], [2.5, 7, 0.5], [3, 0, 1]], Chords: [[0, 0, 2], [0, 2, 2], [0, 4, 2], [2, 3, 2], [2, 5, 2], [2, 7, 2]], Walk: [[0, 0, 1], [1, 2, 1], [2, 4, 1], [3, 5, 1]] };
function degToMidi(d, scale, root) { const sc = BL.SCALES[scale], n = sc.length; return root + 12 * Math.floor(d / n) + sc[((d % n) + n) % n]; }

// =====================================================================================
// Groove: global; swing, accents and drift for every lane
// =====================================================================================
const GKEYS = ['groove', 'complexity', 'forward', 'behind'];
const grooveOps = {
  fresh() { return { groove: clamp(45 + gauss() * 12, 0, 100), complexity: clamp(40 + gauss() * 12, 0, 100), forward: clamp(8 + gauss() * 8, 0, 100), behind: clamp(30 + gauss() * 12, 0, 100), seed: newSeed() }; },
  mutate(from, spread, locks) { const g = clone(from); GKEYS.forEach(k => { if (!locks[k]) g[k] = clamp(g[k] + gauss() * spread * 60, 0, 100); }); g.seed = newSeed(); return g; },
  cross(a, b, locks) { const g = clone(a); GKEYS.forEach(k => { if (!locks[k] && R() < 0.5) g[k] = b[k]; }); return g; },
};
function swingPct(t, g) { return 50 + g.groove / 100 * 16.7 * (t.ease ? clamp(90 / t.bpm, 0.45, 1.25) : 1); }
function grooveNow(t) { const G = t.groove; if (G.use.startsWith('Groove ')) { const k = G.kept.find(x => x && x.name === G.use); if (k) return k.g; } return G.lit.items[L4.indexOf(G.use)] || G.lit.items[0]; }
function feelOf(g, feel) { const f = g.forward / 100 * 30, b = g.behind / 100 * 30; // the Groove pane's rules: ms, brightness, length, level
  if (feel === 'ahead') return { ms: -f, vel: 1, dm: 1 - f / 30 * 0.15, br: 1 + f / 30 * 0.25 };
  if (feel === 'behind') return { ms: b, vel: 1 + b / 30 * 0.3, dm: 1 + b / 30 * 0.35, br: 1 - b / 30 * 0.1 };
  return { ms: 0, vel: 1, dm: 1, br: 1 }; }

// =====================================================================================
// Break writer: a row's part from its pad's label, steered by the loop's traits
// =====================================================================================
const ROLE = { Kick: 'kick', Snare: 'snare', Hats: 'hat', Perc: 'perc', None: 'drum' };
const DTRAITS = ['Tight', 'Dynamic', 'Ghosts', 'Dense', 'Off beat', 'Repeat', 'Fills', 'Anchored'];
const W16 = [0, -4, -3, -4, -2, -4, -3, -4, -1, -4, -3, -4, -2, -4, -3, -4];
function grooveTraits(g) { const c = g.complexity / 100, d = g.groove / 100; return [clamp(0.9 - d * 0.45, 0, 1), clamp(0.3 + d * 0.6, 0, 1), 0.35, clamp(0.15 + c * 0.7, 0, 1), c, 0.7, 0.3, 0.9]; }
function hasOf(roles) { return { kick: roles.includes('kick'), snare: roles.includes('snare'), hat: roles.includes('hat') }; }
function lhl(row) { const N = row.length, on = []; for (let i = 0; i < N; i++) if (row[i] >= 0.45) on.push(i); if (!on.length) return 0; let s = 0;
  for (let n = 0; n < on.length; n++) { const i = on[n], j = on[(n + 1) % on.length] + (n + 1 === on.length ? N : 0); let mw = -9; for (let k = i + 1; k < j; k++) mw = Math.max(mw, W16[k % 16]); if (mw > W16[i % 16]) s += mw - W16[i % 16]; }
  return s / (N / 16); }
function syncTarget(role, c) { return role === 'kick' ? c * 5 : role === 'snare' ? 3 + c * 1.6 : role === 'hat' ? c * 1.6 : c * 3; }
const isAnchor = (role, s) => (role === 'kick' && s % 16 === 0) || (role === 'snare' && (s % 16 === 4 || s % 16 === 12));
function odds(role, c, has, s) {
  const b = s % 16;
  if (role === 'kick') { const P = { 0: 1, 8: 0.8 - 0.5 * c, 10: 0.1 + 0.5 * c, 6: 0.06 + 0.4 * c, 7: 0.04 + 0.4 * c, 3: 0.04 + 0.35 * c, 11: 0.04 + 0.3 * c, 14: 0.06 + 0.35 * c, 15: 0.03 + 0.3 * c, 2: 0.03 + 0.25 * c, 9: 0.02 + 0.2 * c, 13: 0.02 + 0.15 * c, 5: 0.01 + 0.1 * c }; if (!has.snare) { P[4] = 0.15 + 0.2 * c; P[12] = 0.2 + 0.2 * c; } return P[b] || 0; }
  if (role === 'snare') return b === 4 || b === 12 ? 1 : (!has.kick && b === 0 ? 0.6 : 0);
  if (role === 'hat') return b % 4 === 0 ? 1 : b % 2 === 0 ? 1 - 0.2 * c : c * 0.7 * (b % 4 === 3 ? 1 : 0.6);
  return ({ 3: 0.3, 6: 0.25, 7: 0.2, 10: 0.3, 11: 0.25, 13: 0.2, 14: 0.3, 15: 0.35, 2: 0.15, 5: 0.15, 9: 0.15 }[b] || 0.04) * (0.3 + c);
}
const GHOST_W = { 7: 1, 9: 0.8, 15: 0.9, 13: 0.6, 11: 0.7, 3: 0.5, 1: 0.3, 5: 0.3, 14: 0.5, 10: 0.4, 6: 0.4, 2: 0.3 };
function velFor(role, s, dyn) { const b = s % 16, base = role === 'hat' ? (b % 4 === 0 ? 0.9 : b % 2 === 0 ? 0.6 : 0.38) : (b === 0 ? 1 : b % 4 === 0 ? 0.9 : b % 2 === 0 ? 0.72 : 0.62); return clamp(0.78 + (base - 0.78) * dyn * 1.6, 0.15, 1); }
function candidate(role, tr, has, bars) {
  const [, dyn, gh, dense, offb, rep, fills, anch] = tr, N = bars * 16, g = new Array(N).fill(0), densF = 0.35 + dense * 1.3;
  const p = s => isAnchor(role, s) ? 1 : clamp(odds(role, offb, has, s) * densF, 0, 1);
  const anchorOn = () => anch >= 0.5 || R() < anch * 2;
  const bar1 = []; for (let s = 0; s < 16; s++) if (R() < p(s)) bar1.push(s);
  for (let b = 0; b < bars; b++) for (let s = 0; s < 16; s++) {
    let on = b === 0 ? bar1.includes(s) : (R() < (1 - rep) * 0.7 ? R() < p(s) : bar1.includes(s)); // later bars mostly repeat the first
    if (isAnchor(role, s)) on = anchorOn();
    if (on) g[b * 16 + s] = velFor(role, s, dyn);
  }
  for (let s = 0; s < N; s++) {
    if (g[s]) continue;
    if (role === 'snare' && GHOST_W[s % 16] && R() < gh * 0.5 * GHOST_W[s % 16]) g[s] = 0.18 + R() * 0.12;
    else if (role !== 'snare' && role !== 'kick' && s % 2 && R() < gh * 0.12) g[s] = 0.22 + R() * 0.1;
  }
  const fp = role === 'kick' ? fills * 0.3 : role === 'hat' ? fills * 0.4 : fills * 0.75; // a fill into the top
  for (let s = N - 4; s < N; s++) if (!g[s] && R() < fp) g[s] = 0.5 + R() * 0.35;
  return g;
}
function pickBySync(cands, role, c) { const T = syncTarget(role, c); let best = cands[0], bs = 1e9; cands.forEach(g => { const d = Math.abs(lhl(g) - T) + R() * 0.6; if (d < bs) { bs = d; best = g; } }); return best; }
function freshRow(role, tr, has, bars) { return pickBySync(Array.from({ length: 12 }, () => candidate(role, tr, has, bars)), role, tr[4]); }
function mutateRow(row, role, tr, has, spread) {
  const h = row.slice(), N = h.length, n = 1 + poisson(spread * 8 * (N / 32)), c = tr[4], keep = s => tr[7] >= 0.5 && isAnchor(role, s);
  for (let k = 0; k < n; k++) {
    const on = []; for (let s = 0; s < N; s++) if (h[s]) on.push(s);
    const op = R(), free = on.filter(s => !keep(s));
    if (op < 0.38) { const ws = h.map((x, s) => x ? 0 : 0.03 + odds(role, c, has, s) * (0.35 + tr[3] * 1.3)); let t = ws.reduce((a, b) => a + b, 0) * R(), s = 0; for (; s < N - 1; s++) { t -= ws[s]; if (t <= 0) break; } if (!h[s]) h[s] = role === 'snare' && R() < tr[2] * 1.5 ? 0.18 + R() * 0.12 : velFor(role, s, tr[1]); }
    else if (op < 0.66 && free.length) h[free[Math.floor(R() * free.length)]] = 0;
    else if (op < 0.9 && free.length) { const s = free[Math.floor(R() * free.length)], t = (s + (R() < 0.5 ? -1 : 1) + N) % N; if (!h[t]) { h[t] = h[s]; h[s] = 0; } }
    else if (on.length) { const s = on[Math.floor(R() * on.length)]; h[s] = clamp(h[s] + gauss() * spread * tr[1], 0.1, 1); }
  }
  return h;
}
function laneRoles(lane) { return lane.pattern.chosen.map(i => { const p = lane.sound.pads[i]; return p ? ROLE[p.label] : null; }); }
function rowTr(lp, r) { return lp.rowTraits && lp.rowTraits[r] ? lp.rowTraits[r] : lp.traits; }
const mutTraits = (tr, spread, locks) => tr.map((x, i) => locks[i] ? x : fold(x + gauss() * spread * 0.5));
function loopOps(t, lane) {
  const P = lane.pattern, cur = () => P.lit && P.lit.items[P.lit.hear];
  return {
    fresh() {
      const roles = laneRoles(lane), has = hasOf(roles), base = grooveTraits(grooveNow(t)), heard = cur();
      const traits = base.map((x, i) => heard && P.traitLocks[i] ? heard.traits[i] : clamp(x + gauss() * 0.12, 0, 1));
      const lp = { bars: t.bars, traits, rowTraits: heard ? clone(heard.rowTraits) : [null, null, null, null], seed: newSeed(), roles: roles.slice(), rows: [] };
      lp.rows = roles.map((role, r) => heard && P.rowLocks[r] ? heard.rows[r].slice() : role ? freshRow(role, rowTr(lp, r), has, t.bars) : new Array(t.bars * 16).fill(0));
      if (heard) lp.roles = lp.roles.map((ro, r) => P.rowLocks[r] ? heard.roles[r] : ro);
      return lp;
    },
    mutate(from, spread) {
      const roles = laneRoles(lane), has = hasOf(roles), out = clone(from);
      out.traits = mutTraits(from.traits, spread, P.traitLocks);
      out.rowTraits = from.rowTraits.map(x => x ? mutTraits(x, spread, P.traitLocks) : null);
      out.rows = from.rows.map((row, r) => { const role = roles[r]; if (P.rowLocks[r]) return row.slice(); if (!role) return new Array(row.length).fill(0);
        if (from.roles[r] !== role || !row.some(Boolean)) return freshRow(role, rowTr(out, r), has, from.bars); // a relabelled row is written for its new part
        const before = row.join(','), tr = rowTr(out, r), cands = []; for (let k = 0; k < 6; k++) { const h = mutateRow(row, role, tr, has, spread); if (h.join(',') !== before) cands.push(h); }
        return cands.length ? pickBySync(cands, role, tr[4]) : mutateRow(row, role, tr, has, Math.max(spread, 0.2)); });
      out.roles = roles.map((ro, r) => P.rowLocks[r] ? from.roles[r] : ro); out.seed = R() < 0.5 ? from.seed : newSeed();
      return out;
    },
    cross(a, b) { const c = clone(a); c.rows = a.rows.map((row, r) => P.rowLocks[r] || R() < 0.5 || b.bars !== a.bars ? row.slice() : b.rows[r].slice()); c.roles = a.roles.map((ro, r) => c.rows[r] === a.rows[r] ? ro : b.roles[r]); c.traits = a.traits.map((x, i) => P.traitLocks[i] || R() < 0.5 ? x : b.traits[i]); return c; },
  };
}
// Moving a trait redraws the heard loop from its seed (unlocked rows only), as a synth loop redraws from its traits.
function redrawLoop(t, lane, lp) {
  const P = lane.pattern, roles = laneRoles(lane), has = hasOf(roles);
  roles.forEach((role, r) => { if (P.rowLocks[r]) return; lp.rows[r] = role ? withSeed(lp.seed + r * 7919, () => freshRow(role, rowTr(lp, r), has, lp.bars)) : new Array(lp.bars * 16).fill(0); lp.roles[r] = role; });
}

// =====================================================================================
// Pitched loops: the shared engine's trait loops; a bass can follow a kick
// =====================================================================================
const BIAS = { bass: [[0, 0.2], [0, 0.15], [0.2, 0.8], [0, 0.9], [0.2, 0.7], [0, 0.5], [0.2, 0.6], [0.2, 0.8]], synth: [[0, 0.3], [0.2, 0.9], [0, 0.8], [0, 1], [0.2, 0.8], [0.2, 0.9], [0.15, 0.7], [0.3, 1]] };
function cpOf(t, lane) { const [lo, hi] = PRANGE[lane.kind]; return { bars: t.bars, scale: t.scale, lo, hi }; }
function kickBeats(t, id) {
  const d = t.lanes.find(l => l.id === id); if (!d || d.kind !== 'drums') return [];
  const lp = curLoop(d), out = new Set(); lp.rows.forEach((row, r) => { if (lp.roles[r] === 'kick') row.forEach((v, s) => { if (v >= 0.45) out.add(s / 4); }); });
  return [...out].sort((a, b) => a - b);
}
function follow(t, lane, g) {
  const id = lane.pattern.follow; if (lane.kind !== 'bass' || !id) return g;
  const kicks = kickBeats(t, id).filter(b => b < g.bars * 4); if (!kicks.length) return g;
  const c = clone(g), on = (n, b) => Math.abs(n.s - b) < 0.13;
  kicks.forEach(b => {
    if (c.notes.some(n => on(n, b)) || R() > 0.8) return;
    let best = null, bd = 0.76; c.notes.forEach(n => { const d = Math.abs(n.s - b); if (d < bd && !kicks.some(k => on(n, k))) { bd = d; best = n; } });
    if (best) best.s = b; else c.notes.push({ s: b, l: 0.5, p: c.notes.length ? Math.min(...c.notes.map(n => n.p)) : PRANGE.bass[0] + 7, v: 0.85 });
  });
  c.notes.sort((a, b) => a.s - b.s);
  const mono = []; c.notes.forEach(n => { const last = mono[mono.length - 1]; if (last && Math.abs(last.s - n.s) < 1e-6) return; if (last && last.s + last.l > n.s) last.l = Math.max(1 / 16, n.s - last.s); mono.push(n); });
  c.notes = mono; return c;
}
function notesOps(t, lane) { const cp = cpOf(t, lane); return {
  fresh: () => follow(t, lane, BL.synthFresh(cp, BIAS[lane.kind])),
  mutate: (g, s) => follow(t, lane, BL.synthMutate(g, s, cp, null)),
  cross: (a, b) => follow(t, lane, BL.synthCross(a, b, cp, null)),
}; }
function redrawNotes(t, lane, g) { return follow(t, lane, BL.synthRedraw(g, cpOf(t, lane))); }

// =====================================================================================
// Litters: breed, again, cross, back, new (history kept outside the state, by litter id)
// =====================================================================================
let litN = 0;
const HIST = new Map();
function litter(items) { return { id: 'L' + (++litN) + '-' + Math.floor(R() * 1e6), items, hear: 0, parent: -1, spread: TUNE.start, streak: 0 }; }
const hist = o => { if (!HIST.has(o.id)) HIST.set(o.id, []); return HIST.get(o.id); };
function snap(o) { const h = hist(o); h.push(clone({ items: o.items, hear: o.hear, parent: o.parent, spread: o.spread, streak: o.streak })); if (h.length > 40) h.shift(); }
function litBreed(o, i, mutate) { snap(o); if (i === o.parent) { o.spread *= TUNE.narrow; o.streak = 0; } else if (++o.streak >= 2) { o.spread *= TUNE.widen; o.streak = 0; } o.spread = clamp(o.spread, TUNE.min, TUNE.max); const p = o.items[i]; o.items = o.items.map((x, k) => k === i ? p : mutate(p, o.spread)); o.parent = i; o.hear = i; }
function litCross(o, i, cross, mutate) { const j = o.hear; if (i === j) return litBreed(o, i, mutate); snap(o); const a = o.items[i], b = o.items[j]; o.items = o.items.map((x, k) => k === i || k === j ? x : mutate(cross(a, b), o.spread * 0.5)); o.parent = i; }
function litBack(o) { const s = hist(o).pop(); if (!s) return false; Object.assign(o, s); return true; }
function litNew(o, fresh) { snap(o); const items = []; for (let k = 0; k < 4; k++) items.push(fresh()); o.items = items; o.parent = -1; o.hear = 0; o.spread = TUNE.start; o.streak = 0; }
function relit(x) { if (x && typeof x === 'object') { if (typeof x.id === 'string' && x.id[0] === 'L' && Array.isArray(x.items)) { const old = HIST.get(x.id); x.id = 'L' + (++litN) + '-' + Math.floor(R() * 1e6); if (old) HIST.set(x.id, clone(old)); } Object.values(x).forEach(relit); } }

// =====================================================================================
// Track state
// =====================================================================================
const emptyChain = () => ({ char: 'None', mix: 0.6, fx: [] });
function fxNew() { return { lit: litter([emptyChain(), emptyChain(), emptyChain(), emptyChain()]), bypass: false, char: 'None' }; }
const curChain = F => F.lit.items[F.lit.hear];
const curLoop = lane => lane.pattern.lit.items[lane.pattern.lit.hear];
const curVoice = lane => lane.sound.lit.items[lane.sound.lit.hear];
function laneBase(t, kind, name) { return { id: t.nextId++, name, kind, db: kind === 'drums' ? -6 : kind === 'bass' ? -8 : -10, pan: 0, mute: false, solo: false, feel: 'on', followSwing: true, levelLocked: false, fx: fxNew() }; }
function drumLane(t, name, kit) {
  const l = laneBase(t, 'drums', name);
  l.sound = { zone: 'Kick', lit: litter([0, 1, 2, 3].map(() => drumOps.fresh('Kick'))), locks: Array(19).fill(false), pads: Array(8).fill(null) };
  (kit || []).forEach(([z, n, lvl], i) => { const v = drumOps.fresh(z); if (lvl != null) v[LEVEL] = norm(LEVEL, lvl); l.sound.pads[i] = { name: n, zone: z, label: z, v }; });
  l.pattern = { chosen: [0, 1, 2, 3], lit: null, rowLocks: [false, false, false, false], rowFeel: ['on', 'behind', 'on', 'ahead'], traitScope: -1, traitLocks: Array(8).fill(false), kept: [null, null, null, null] };
  const ops = loopOps(t, l); l.pattern.lit = litter([]); l.pattern.lit.items = [0, 1, 2, 3].map(() => ops.fresh());
  return l;
}
function pitchedLane(t, kind, name) {
  const l = laneBase(t, kind, name), zone = kind === 'bass' ? 'Round' : 'Keys';
  if (kind === 'synth') l.pan = -0.2;
  if (kind === 'bass') l.feel = 'behind';
  l.sound = { zone, lit: litter([0, 1, 2, 3].map(() => voiceOps.fresh(kind, zone))), locks: Array(SY.length).fill(false), kept: [null, null, null, null], phrase: 'Root and fifth' };
  const bassKick = kind === 'bass' ? (t.lanes.find(x => x.kind === 'drums') || {}).id || 0 : 0;
  l.pattern = { follow: bassKick, lit: litter([]), kept: [null, null, null, null] };
  const ops = notesOps(t, l); l.pattern.lit.items = [0, 1, 2, 3].map(() => ops.fresh());
  return l;
}
function createTrack(opts = {}) {
  const t = { bpm: 92, bars: 2, key: 'A', scale: 'Minor', ease: true, nextId: 1,
    groove: { lit: litter([0, 1, 2, 3].map(() => grooveOps.fresh())), locks: { groove: false, complexity: false, forward: false, behind: false }, kept: [null, null, null, null], use: 'A' },
    master: { db: 0, fx: fxNew() }, lanes: [], adding: null, draftAlone: false, undo: [] };
  if (opts.empty) return t;
  t.lanes.push(drumLane(t, 'Break', [['Kick', 'Kick 1'], ['Snare', 'Snare 1'], ['Hats', 'Hats 1', -8], ['None', 'Drum 1', -6]]));
  t.lanes.push(pitchedLane(t, 'bass', 'Sub line'));
  t.lanes.push(pitchedLane(t, 'synth', 'Keys'));
  if (!opts.dry) { // effects on the example track
    const keys = t.lanes[2].fx; keys.char = 'Space'; litNew(keys.lit, () => BL.fxFresh('Space')); HIST.delete(keys.lit.id);
    const m = t.master.fx; m.char = 'Dirt'; m.lit.items = m.lit.items.map(() => ({ char: 'Dirt', mix: 0.35, fx: [{ type: 'tape', p: [0.15, 0.3, 0.1, 0.35], lock: false }, { type: 'comp', p: [0.7, 0.15, 0.3, 0.4], lock: false }] }));
  }
  return t;
}
const laneById = (t, id) => t.lanes.find(l => l.id === id) || (t.adding && t.adding.draft.id === id ? t.adding.draft : null);

// ---------- Track edits (undoable)
function snapshot(t, what) { t.undo.push({ what, lanes: clone(t.lanes), master: clone(t.master) }); if (t.undo.length > 40) t.undo.shift(); }
function undo(t) { const u = t.undo.pop(); if (!u) return null; t.lanes = u.lanes; t.master = u.master; return u.what; }
function setDb(t, lane, db) { lane.db = db; }
function setPan(t, lane, p) { lane.pan = clamp(p, -1, 1); }
function toggle(t, lane, key) { snapshot(t, key === 'mute' ? 'mute' : key === 'solo' ? 'solo' : key); lane[key] = !lane[key]; }
function relabel(t, lane, padIndex, label) { const p = lane.sound.pads[padIndex]; if (!p || p.label === label) return false; snapshot(t, 'relabel'); p.label = label; return true; }
function rewriteRow(t, lane, r) {
  const P = lane.pattern, pad = lane.sound.pads[P.chosen[r]]; if (!pad || P.rowLocks[r]) return false;
  const lp = curLoop(lane), roles = laneRoles(lane); snap(P.lit);
  lp.rows[r] = freshRow(ROLE[pad.label], rowTr(lp, r), hasOf(roles), lp.bars); lp.roles[r] = ROLE[pad.label]; return true;
}
function staleRows(lane) { const lp = curLoop(lane), roles = laneRoles(lane); return roles.map((ro, r) => !!ro && lp.roles[r] !== ro); }
function removeLane(t, lane) { snapshot(t, 'remove'); t.lanes = t.lanes.filter(l => l !== lane); }
function duplicateLane(t, lane) { snapshot(t, 'duplicate'); const c = clone(lane); c.id = t.nextId++; c.name = lane.name + ' 2'; c.solo = false; relit(c); t.lanes.splice(t.lanes.indexOf(lane) + 1, 0, c); return c; }
function moveLane(t, lane, d) { const i = t.lanes.indexOf(lane), j = i + d; if (j < 0 || j >= t.lanes.length) return; snapshot(t, 'move'); t.lanes.splice(i, 1); t.lanes.splice(j, 0, lane); }
function renameLane(t, lane, name) { if (!name.trim() || name === lane.name) return; snapshot(t, 'rename'); lane.name = name.trim(); }
function addPart(t, kind) {
  const n = t.lanes.filter(x => x.kind === kind).length + 1, name = (kind === 'drums' ? 'Drums ' : kind === 'bass' ? 'Bass ' : 'Synth ') + n;
  const draft = kind === 'drums' ? drumLane(t, name, []) : pitchedLane(t, kind, name);
  t.adding = { kind, draft }; return draft;
}
function cancelAdd(t) { t.adding = null; }
function addToTrack(t) { if (!t.adding) return null; snapshot(t, 'add part'); const d = t.adding.draft; t.lanes.push(d); t.adding = null; return d; }
function keepPad(t, lane, i) { const S = lane.sound, z = S.zone, base = z === 'None' ? 'Drum' : z, n = S.pads.filter(p => p && p.name.startsWith(base + ' ')).length + 1; S.pads[i] = { name: base + ' ' + n, zone: z, label: z, v: curVoice(lane).slice() }; return S.pads[i]; }
function setBars(t, n) {
  if (n === t.bars) return; t.bars = n;
  const rs = lp => { if (lp.bars === n) return lp; const c = clone(lp), N = n * 16; c.rows = lp.rows.map(row => Array.from({ length: N }, (_, s) => row[s % row.length])); c.bars = n; return c; };
  t.lanes.concat(t.adding ? [t.adding.draft] : []).forEach(l => {
    const P = l.pattern;
    if (l.kind === 'drums') { P.lit.items = P.lit.items.map(rs); P.kept = P.kept.map(k => k && rs(k)); }
    else { const cp = cpOf(t, l), f = g => g.bars === n ? g : follow(t, l, BL.synthResize(g, n, cp)); P.lit.items = P.lit.items.map(f); P.kept = P.kept.map(k => k && f(k)); }
  });
}
function setScale(t, s) { if (s === t.scale) return; t.scale = s; t.lanes.concat(t.adding ? [t.adding.draft] : []).forEach(l => { if (l.kind !== 'drums') l.pattern.lit.items = l.pattern.lit.items.map(g => redrawNotes(t, l, g)); }); }
function setFollow(t, lane, id) { lane.pattern.follow = id; const L = lane.pattern.lit; L.items[L.hear] = follow(t, lane, L.items[L.hear]); }

// ---------- litters by name
function litFor(t, lane, which) {
  if (which === 'groove') { const G = t.groove; return { o: G.lit, ops: { fresh: () => grooveOps.fresh(), mutate: (p, s) => grooveOps.mutate(p, s, G.locks), cross: (a, b) => grooveOps.cross(a, b, G.locks) } }; }
  if (which === 'fx') { const F = lane === 'master' ? t.master.fx : lane.fx; return { o: F.lit, ops: { fresh: () => BL.fxFresh(F.char), mutate: (p, s) => BL.fxMutate(p, s, false), cross: BL.fxCross } }; }
  const S = lane.sound;
  if (which === 'sound') return lane.kind === 'drums'
    ? { o: S.lit, ops: { fresh: () => drumOps.fresh(S.zone), mutate: (p, s) => drumOps.mutate(p, s, S.locks, S.zone), cross: (a, b) => drumOps.cross(a, b, S.locks) } }
    : { o: S.lit, ops: { fresh: () => voiceOps.fresh(lane.kind, S.zone), mutate: (p, s) => voiceOps.mutate(p, s, S.locks, lane.kind, S.zone), cross: (a, b) => voiceOps.cross(a, b, S.locks) } };
  return { o: lane.pattern.lit, ops: lane.kind === 'drums' ? loopOps(t, lane) : notesOps(t, lane) };
}
function breed(t, lane, which, i) { const { o, ops } = litFor(t, lane, which); litBreed(o, i, ops.mutate); }
function cross(t, lane, which, i) { const { o, ops } = litFor(t, lane, which); litCross(o, i, ops.cross, ops.mutate); }
function back(t, lane, which) { return litBack(litFor(t, lane, which).o); }
function renew(t, lane, which) { const { o, ops } = litFor(t, lane, which); litNew(o, ops.fresh); }

// ---------- effects edits
const FX_TYPES = Object.keys(BL.FX_TYPES);
function fxAdd(F, type) { const c = curChain(F); if (c.fx.length >= 6) return false; if (!c.fx.length) c.mix = 1; c.fx.push({ type, p: [0.5, 0.35, 0.5, 0.4], lock: false }); c.fx.sort((a, b) => (a.type === 'reverb') - (b.type === 'reverb')); return true; }
function fxRemove(F, k) { curChain(F).fx.splice(k, 1); }
function fxMove(F, k, d) { const fx = curChain(F).fx, j = k + d; if (j < 0 || j >= fx.length) return; [fx[k], fx[j]] = [fx[j], fx[k]]; }

// =====================================================================================
// Render
// =====================================================================================
const loopLen = t => Math.round(t.bars * 16 * 60 / t.bpm / 4 * SR);
const TRIM = 0.6; // about -4.4 dB a drum row, so four pads sit under the limiter
function addWrapped(dst, src, start, g) { const L = dst.length; for (let q = 0; q < src.length; q++) { let j = (start + q) % L; if (j < 0) j += L; dst[j] += src[q] * g; } }
function drumDry(t, lane, Ls, g) {
  const lp = curLoop(lane), P = lane.pattern, sw = lane.followSwing ? swingPct(t, g) / 100 : 0.5, sd = 60 / t.bpm / 4, l = new Float32Array(Ls), r = new Float32Array(Ls), lf = feelOf(g, lane.feel);
  lp.rows.forEach((row, ri) => {
    const pad = lane.sound.pads[P.chosen[ri]]; if (!pad) return;
    const role = lp.roles[ri] || ROLE[pad.label], tr = rowTr(lp, ri), tight = tr[0], dyn = tr[1], rf = feelOf(g, P.rowFeel[ri]);
    const depth = g.groove / 100 * 7 * (0.4 + dyn * 1.2), sig = g.groove / 100 * 5 * (1.8 - 1.6 * tight);
    const rr = mulberry32((g.seed || 1) * 31 + ri * 17 + lp.seed + lane.id * 101), contour = [], drift = [];
    for (let k = 0; k < lp.bars * 4; k++) contour.push(k % 4 === 0 ? 1 : 1 - rr() * 0.3 * g.groove / 100);
    let x = 0, lastJ = 0; for (let s = 0; s < row.length; s++) { x = 0.9 * x + 0.45 * gaussOf(rr); const j = gaussOf(rr) * 0.4 - 0.3 * lastJ; lastJ = j; drift.push((x * 0.7 + j) * sig); }
    const hits = [];
    row.forEach((v, s) => { if (!v) return;
      let tt = s * sd; if (s % 2) tt += (sw * 2 - 1) * sd;
      const lvl = s % 4 === 0 ? 0 : s % 2 === 0 ? 0.5 : 1;
      const ms = lf.ms + rf.ms + (role === 'kick' ? 0 : drift[s]); // the kick stays on time
      hits.push({ s, t: tt + ms / 1000, vel: clamp(v * dbg(-depth * lvl) * contour[Math.floor(s / 4)] * lf.vel * rf.vel, 0.05, 1), dm: lf.dm * rf.dm, br: lf.br * rf.br });
    });
    hits.sort((a, b) => a.t - b.t);
    const pan = real(PAN, pad.v[PAN]), gl = Math.min(1, 1 - pan) * TRIM, gr = Math.min(1, 1 + pan) * TRIM, lev = real(LEVEL, pad.v[LEVEL]);
    hits.forEach((h, k) => {
      const start = Math.round(h.t * SR); let cut = Ls;
      if (hits.length > 1) { const nx = hits[(k + 1) % hits.length]; cut = Math.round(nx.t * SR) - start; if (cut <= 0) cut += Ls; } // one voice per pad
      const s = drumVoice(pad.v, { vel: h.vel, dm: h.dm, br: h.br, levelDb: lev }, (lp.seed + ri * 7919 + h.s * 104729) >>> 0, cut);
      addWrapped(l, s, start, gl); addWrapped(r, s, start, gr);
    });
  });
  return { l, r };
}
function pitchDry(t, lane, Ls, g) {
  const lp = curLoop(lane), vn = curVoice(lane), sw = lane.followSwing ? swingPct(t, g) / 100 : 0.5, beat = 60 / t.bpm, l = new Float32Array(Ls);
  const ko = KEYS.indexOf(t.key), koff = ko > 6 ? ko - 12 : ko, f = feelOf(g, lane.feel), depth = g.groove / 100 * 7 * 0.6, rr = mulberry32((g.seed || 1) * 13 + lane.id * 101 + (lp.seed || 1));
  lp.notes.forEach(n => {
    const p16 = n.s * 4, w = lane.followSwing ? BL.swingWarp(p16, sw) : p16, frac = Math.abs(p16 - Math.round(p16)) < 0.01 ? Math.round(p16) : -1;
    const lvl = frac < 0 ? 1 : frac % 4 === 0 ? 0 : frac % 2 === 0 ? 0.5 : 1;
    const ms = f.ms + gaussOf(rr) * g.groove / 100 * 2;
    const s = synthVoice(vn, n.p + koff, n.l * beat * f.dm, clamp(n.v * dbg(-depth * lvl) * f.vel, 0.05, 1));
    addWrapped(l, s, Math.round((w / 4 * beat + ms / 1000) * SR), 1);
  });
  return { l, r: l.slice() };
}
function phrase(t, lane, vn, name) { // a short audition in the Track's key
  const ph = PHRASES[name] || PHRASES['Root and fifth'], beat = 60 / t.bpm, ko = KEYS.indexOf(t.key), [lo, hi] = PRANGE[lane.kind];
  const root = BL.foldInto(Math.round((lo + (hi - lo) * 0.3) / 12) * 12 + ko, lo, hi), N = Math.round((4 * beat + 1.5) * SR), out = new Float32Array(N);
  ph.forEach(([s, d, len]) => { const v = synthVoice(vn, degToMidi(d, t.scale, root), len * beat * 0.9, 0.85), st = Math.round(s * beat * SR); for (let q = 0; q < v.length && st + q < N; q++) out[st + q] += v[q]; });
  return out;
}
function rms(l, r) { let s = 0; for (let i = 0; i < l.length; i++) s += l[i] * l[i] + r[i] * r[i]; return Math.sqrt(s / (2 * l.length || 1)); }
function blocks(l, r, n = 1024) { const out = new Float32Array(Math.ceil(l.length / n)); for (let i = 0; i < l.length; i++) { const a = Math.max(Math.abs(l[i]), Math.abs(r[i])); const k = (i / n) | 0; if (a > out[k]) out[k] = a; } return out; }
function limit(left, right) { // the Bloodline Effects safety: a -1 dB limiter, run twice round so the loop's start is settled
  const n = left.length, rel = Math.exp(-1 / (0.08 * SR)); let lim = 1, peak = 0, cut = 1;
  for (let pass = 0; pass < 2; pass++) for (let i = 0; i < n; i++) { const pk = Math.max(Math.abs(left[i]), Math.abs(right[i])), want = pk > 0.891 ? 0.891 / pk : 1; lim = want < lim ? want : want + (lim - want) * rel; if (pass) { left[i] = clamp(left[i] * lim, -1, 1); right[i] = clamp(right[i] * lim, -1, 1); peak = Math.max(peak, pk); cut = Math.min(cut, lim); } }
  return { peak, cut };
}
// Seamless effects: feed the loop round enough times for tails to settle, keep the last pass.
async function loopFx(fxFn, chain, l, r, bpm) {
  const L = l.length, pre = Math.max(1, Math.ceil(5 * SR / L)), n = (pre + 1) * L, tl = new Float32Array(n), tr = new Float32Array(n);
  for (let k = 0; k <= pre; k++) { tl.set(l, k * L); tr.set(r, k * L); }
  const [ol, or] = await fxFn(chain, false, tl, tr, SR, bpm);
  return { l: ol.slice(pre * L, n), r: or.slice(pre * L, n) };
}
const DRY = new Map(), WET = new Map();
const chainLive = F => { const c = curChain(F); return !F.bypass && c && c.fx.length ? c : null; };
function audibleLanes(t) { const all = t.lanes.concat(t.adding ? [t.adding.draft] : []); if (t.adding && t.draftAlone) return new Set([t.adding.draft.id]); const solo = all.some(l => l.solo); return new Set(all.filter(l => !l.mute && (!solo || l.solo)).map(l => l.id)); }
async function render(t, opts = {}) {
  const Ls = loopLen(t), g = grooveNow(t), lanes = t.lanes.concat(t.adding ? [t.adding.draft] : []), hear = audibleLanes(t), fxFn = opts.fx || null;
  const left = new Float32Array(Ls), right = new Float32Array(Ls), stems = [], wet = {}, mg = dbg(t.master.db);
  for (const lane of lanes) {
    const swing = lane.followSwing ? swingPct(t, g) : 50;
    const dk = JSON.stringify(lane.kind === 'drums'
      ? [t.bpm, Ls, g, swing, lane.id, lane.feel, lane.pattern.rowFeel, lane.pattern.chosen.map(i => lane.sound.pads[i] && lane.sound.pads[i].v), lane.pattern.chosen.map(i => lane.sound.pads[i] && lane.sound.pads[i].label), curLoop(lane)]
      : [t.bpm, Ls, g, swing, lane.id, lane.feel, t.key, t.scale, curVoice(lane), curLoop(lane)]);
    let d = DRY.get(lane.id); if (!d || d.key !== dk) { d = { key: dk, ...(lane.kind === 'drums' ? drumDry(t, lane, Ls, g) : pitchDry(t, lane, Ls, g)) }; DRY.set(lane.id, d); }
    const ch = fxFn ? chainLive(lane.fx) : null, wk = dk + JSON.stringify(ch);
    let w = WET.get(lane.id); if (!w || w.key !== wk) { w = { key: wk, ...(ch ? await loopFx(fxFn, ch, d.l, d.r, t.bpm) : { l: d.l, r: d.r }) }; WET.set(lane.id, w); }
    wet[lane.id] = w;
    const on = hear.has(lane.id), gain = on ? dbg(lane.db) * mg : 0, gl = gain * Math.min(1, 1 - lane.pan), gr = gain * Math.min(1, 1 + lane.pan);
    const sl = new Float32Array(Ls), sr = new Float32Array(Ls);
    if (gain) for (let i = 0; i < Ls; i++) { sl[i] = w.l[i] * gl; sr[i] = w.r[i] * gr; left[i] += sl[i]; right[i] += sr[i]; }
    stems.push({ id: lane.id, name: lane.name, kind: lane.kind, l: sl, r: sr, env: blocks(sl, sr), draft: !!(t.adding && t.adding.draft === lane) });
  }
  let ml = left, mr = right; const sumPeak = Math.max(...blocks(left, right));
  const mch = fxFn ? chainLive(t.master.fx) : null;
  if (mch) { const sumL = new Float32Array(Ls), sumR = new Float32Array(Ls); for (let i = 0; i < Ls; i++) { sumL[i] = left[i] / (mg || 1); sumR[i] = right[i] / (mg || 1); } const o = await loopFx(fxFn, mch, sumL, sumR, t.bpm); ml = o.l; mr = o.r; for (let i = 0; i < Ls; i++) { ml[i] *= mg; mr[i] *= mg; } }
  const pre = { l: ml.slice(), r: mr.slice() }, { peak, cut } = limit(ml, mr);
  return { L: Ls, left: ml, right: mr, stems, wet, peak, cut, env: blocks(ml, mr), prePeak: Math.max(...blocks(pre.l, pre.r)), sumPeak };
}
// Auto level: unlocked lanes to a common loudness, then all of them trimmed together so the master peak sits under the limiter.
// Master effects can add gain (a compressor's makeup), so the last render's gain through them is allowed for;
// call it again on a fresh render if the limiter still works.
function autoLevel(t, res, again) {
  const free = t.lanes.filter(l => !l.levelLocked && res.wet[l.id]); if (!free.length) return 0;
  if (!again) snapshot(t, 'auto level');
  const fxGain = res.sumPeak > 1e-6 ? Math.max(0.25, res.prePeak / res.sumPeak) : 1;
  if (again) { const trim = 20 * Math.log10(Math.min(1, res.cut)) - 0.8; free.forEach(l => { l.db = clamp(Math.round((l.db + trim) * 10) / 10, -40, 12); }); return free.length; }
  const target = { drums: 0.08, bass: 0.1, synth: 0.06 };
  free.forEach(l => { const w = res.wet[l.id], x = rms(w.l, w.r); if (x > 1e-6) l.db = clamp(Math.round(20 * Math.log10(target[l.kind] / x) * 10) / 10, -40, 12); });
  const hear = audibleLanes(t), Ls = res.L, sl = new Float32Array(Ls), sr = new Float32Array(Ls), fl = new Float32Array(Ls), fr = new Float32Array(Ls), mg = dbg(t.master.db);
  t.lanes.forEach(l => { const w = res.wet[l.id]; if (!w || !hear.has(l.id)) return; const g = dbg(l.db) * mg, gl = g * Math.min(1, 1 - l.pan), gr = g * Math.min(1, 1 + l.pan), dl = free.includes(l) ? fl : sl, dr = free.includes(l) ? fr : sr; for (let i = 0; i < Ls; i++) { dl[i] += w.l[i] * gl; dr[i] += w.r[i] * gr; } });
  let lo = 0, hi = 1; const want = dbg(-1.5) / fxGain, peakAt = k => { let p = 0; for (let i = 0; i < Ls; i++) p = Math.max(p, Math.abs(sl[i] + fl[i] * k), Math.abs(sr[i] + fr[i] * k)); return p; };
  if (peakAt(1) > want) { for (let it = 0; it < 18; it++) { const m = (lo + hi) / 2; if (peakAt(m) > want) hi = m; else lo = m; } const trim = Math.round(20 * Math.log10(Math.max(lo, 1e-4)) * 10) / 10; free.forEach(l => { l.db = clamp(l.db + trim, -40, 12); }); }
  return free.length;
}

// ---------- export
function wav(chans) {
  const n = chans[0].length, nc = chans.length, ab = new ArrayBuffer(44 + n * nc * 2), v = new DataView(ab), ws = (o, s) => { for (let i = 0; i < s.length; i++) v.setUint8(o + i, s.charCodeAt(i)); };
  ws(0, 'RIFF'); v.setUint32(4, 36 + n * nc * 2, true); ws(8, 'WAVE'); ws(12, 'fmt '); v.setUint32(16, 16, true); v.setUint16(20, 1, true); v.setUint16(22, nc, true); v.setUint32(24, SR, true); v.setUint32(28, SR * nc * 2, true); v.setUint16(32, nc * 2, true); v.setUint16(34, 16, true); ws(36, 'data'); v.setUint32(40, n * nc * 2, true);
  let o = 44; for (let i = 0; i < n; i++) for (let c = 0; c < nc; c++) { v.setInt16(o, Math.round(clamp(chans[c][i], -1, 1) * 32767), true); o += 2; }
  return new Uint8Array(ab);
}
function trackJSON(t) {
  const g = grooveNow(t);
  return { bpm: t.bpm, bars: t.bars, key: t.key, scale: t.scale, lessSwingAtFasterTempos: t.ease, groove: { ...g, swing: swingPct(t, g) },
    master: { db: t.master.db, effects: t.master.fx.bypass ? null : curChain(t.master.fx) },
    lanes: t.lanes.map(l => ({ name: l.name, kind: l.kind, db: l.db, pan: l.pan, mute: l.mute, solo: l.solo, feel: l.feel, followSwing: l.followSwing, effects: l.fx.bypass ? null : curChain(l.fx),
      sound: l.kind === 'drums' ? l.pattern.chosen.map((i, r) => ({ row: r + 1, pad: l.sound.pads[i] && { name: l.sound.pads[i].name, label: l.sound.pads[i].label, zone: l.sound.pads[i].zone, settings: l.sound.pads[i].v } })) : { zone: l.sound.zone, settings: curVoice(l) },
      pattern: l.kind === 'drums' ? { rows: curLoop(l).rows, roles: curLoop(l).roles, traits: curLoop(l).traits, rowFeel: l.pattern.rowFeel } : { notes: curLoop(l).notes, traits: curLoop(l).traits } })) };
}

const BTrack = {
  SR, KEYS, SCALES, LABELS, ROLE, SET, GROUPS, PAN, LEVEL, DTRAITS, FX_TYPES, ZONE_NAMES, PRANGE, PHRASES, L4,
  seed, withSeed, clamp, dbg, clone, real, norm, fmt, swingPct, grooveNow, grooveTraits, isAnchor,
  createTrack, laneById, curLoop, curVoice, curChain, laneRoles, staleRows, rowTr,
  snapshot, undo, setDb, setPan, toggle, relabel, rewriteRow, removeLane, duplicateLane, moveLane, renameLane,
  addPart, cancelAdd, addToTrack, keepPad, setBars, setScale, setFollow, breed, cross, back, renew, litFor, snap,
  redrawLoop, redrawNotes, fxAdd, fxRemove, fxMove, drumVoice, synthVoice, phrase, render, autoLevel, rms, wav, trackJSON, loopLen,
};
global.BTrack = BTrack;
if (typeof module !== 'undefined' && module.exports) module.exports = BTrack;
})(typeof window !== 'undefined' ? window : globalThis);
