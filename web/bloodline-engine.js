/* Bloodline engine. Shared by Bloodline Studio and Bloodline Suite.
   Web Audio sketches of the three plugins (drum voice, synth voice, effects chain),
   the breeding loop (a litter of four, breed, cross, back, spread), drum loops,
   synth loops drawn from eight traits, and a look-ahead transport. */
(function (global) {
  'use strict';
  const BL = {};

  // ---------- maths and randomness
  const clamp = (x, a, b) => (x < a ? a : x > b ? b : x);
  const lerp = (a, b, t) => a + (b - a) * t;
  const fold01 = x => { x = ((x % 2) + 2) % 2; return x > 1 ? 2 - x : x; };
  const mod = (a, n) => ((a % n) + n) % n;
  function rng(seed) {
    let s = (seed >>> 0) || 0x9e3779b9;
    return function () { s ^= s << 13; s >>>= 0; s ^= s >>> 17; s ^= s << 5; s >>>= 0; return s / 4294967296; };
  }
  function gaussOf(r) { let u = 0; while (!u) u = r(); return Math.sqrt(-2 * Math.log(u)) * Math.cos(2 * Math.PI * r()); }
  const R = Math.random;
  const gauss = () => gaussOf(R);
  const pick = (arr, r = R) => arr[Math.floor(r() * arr.length)];
  function weighted(ws, r = R) {
    let t = 0; for (const w of ws) t += w;
    if (t <= 0) return -1;
    let x = r() * t;
    for (let i = 0; i < ws.length; i++) { x -= ws[i]; if (x <= 0) return i; }
    return ws.length - 1;
  }
  function poisson(l, r = R) { const L = Math.exp(-l); let k = 0, p = 1; do { k++; p *= r(); } while (p > L); return k - 1; }
  const newSeed = () => Math.floor(R() * 2147483647);
  const clone = o => JSON.parse(JSON.stringify(o));
  Object.assign(BL, { clamp, lerp, fold01, mod, rng, gauss, gaussOf, pick, weighted, poisson, newSeed, clone });

  // ---------- settings
  const WAVES = ['sine', 'triangle', 'sawtooth', 'square'];
  const WAVE_NAMES = ['sine', 'tri', 'saw', 'square'];
  const S = (name, min, max, curve, unit, group) => ({ name, min, max, curve, unit, group });
  const DRUM = [
    S('Pitch', 30, 900, 'exp', 'Hz', 'Body'), S('Sweep', 0, 36, 'lin', 'st', 'Body'),
    S('Sweep time', 4, 150, 'exp', 'ms', 'Body'), S('Decay', 20, 1200, 'exp', 'ms', 'Body'),
    S('Noise', 0, 1, 'lin', '%', 'Noise'), S('Noise tone', 300, 14000, 'exp', 'Hz', 'Noise'),
    S('Noise decay', 10, 600, 'exp', 'ms', 'Noise'), S('Metal', 0, 1, 'lin', '%', 'Noise'),
    S('Click', 0, 1, 'lin', '%', 'Grit'), S('Drive', 0, 1, 'lin', '%', 'Grit'),
  ];
  const SYNTH = [
    S('Shape A', 0, 1, 'lin', 'wave', 'Osc'), S('Shape B', 0, 1, 'lin', 'wave', 'Osc'),
    S('Mix B', 0, 1, 'lin', '%', 'Osc'), S('Tune B', -12, 12, 'lin', 'semi', 'Osc'),
    S('Detune', 0, 40, 'lin', 'ct', 'Osc'), S('Sub', 0, 1, 'lin', '%', 'Osc'),
    S('Cutoff', 80, 12000, 'exp', 'Hz', 'Filter'), S('Resonance', 0.5, 16, 'exp', 'Q', 'Filter'),
    S('Env amount', 0, 5, 'lin', 'oct', 'Filter'), S('Filter decay', 20, 2000, 'exp', 'ms', 'Filter'),
    S('Attack', 1, 1500, 'exp', 'ms', 'Amp'), S('Decay', 30, 3000, 'exp', 'ms', 'Amp'),
    S('Sustain', 0, 1, 'lin', '%', 'Amp'), S('Release', 10, 3000, 'exp', 'ms', 'Amp'),
    S('Vibrato', 0, 0.6, 'lin', 'st', 'Motion'), S('LFO rate', 0.2, 10, 'exp', 'Hz', 'Motion'),
    S('Drive', 0, 1, 'lin', '%', 'Motion'),
  ];
  function toReal(s, n) { return s.curve === 'exp' ? s.min * Math.pow(s.max / s.min, n) : s.min + (s.max - s.min) * n; }
  function toNorm(s, v) { return clamp(s.curve === 'exp' ? Math.log(v / s.min) / Math.log(s.max / s.min) : (v - s.min) / (s.max - s.min), 0, 1); }
  function fmt(s, n) {
    const v = toReal(s, n);
    switch (s.unit) {
      case 'Hz': return v >= 1000 ? (v / 1000).toFixed(1) + ' kHz' : (v < 10 ? v.toFixed(1) : Math.round(v)) + ' Hz';
      case 'ms': return v >= 1000 ? (v / 1000).toFixed(2) + ' s' : Math.round(v) + ' ms';
      case '%': return Math.round(v * 100) + '%';
      case 'st': return v.toFixed(1) + ' st';
      case 'semi': return (Math.round(v) > 0 ? '+' : '') + Math.round(v) + ' st';
      case 'ct': return Math.round(v) + ' ct';
      case 'oct': return v.toFixed(1) + ' oct';
      case 'Q': return v.toFixed(1);
      case 'wave': return WAVE_NAMES[Math.min(3, Math.floor(n * 4))];
      default: return v.toFixed(2);
    }
  }

  // Zones, in real units. Anything not listed spans the whole range.
  const DRUM_ZONES = {
    None: {},
    Kick: { Pitch: [40, 75], Sweep: [12, 36], 'Sweep time': [15, 80], Decay: [150, 700], Noise: [0, 0.15], 'Noise tone': [300, 3000], 'Noise decay': [10, 60], Metal: [0, 0.04], Click: [0.1, 0.6], Drive: [0, 0.5] },
    Snare: { Pitch: [150, 280], Sweep: [2, 12], 'Sweep time': [10, 50], Decay: [60, 220], Noise: [0.45, 0.85], 'Noise tone': [1500, 8000], 'Noise decay': [80, 300], Metal: [0, 0.2], Click: [0.1, 0.5], Drive: [0, 0.5] },
    Hats: { Pitch: [300, 900], Sweep: [0, 2], 'Sweep time': [4, 20], Decay: [20, 80], Noise: [0.85, 1], 'Noise tone': [6500, 14000], 'Noise decay': [15, 320], Metal: [0.3, 0.9], Click: [0, 0.3], Drive: [0, 0.3] },
    Perc: { Pitch: [200, 900], Sweep: [0, 6], 'Sweep time': [5, 40], Decay: [20, 200], Noise: [0.2, 0.6], 'Noise tone': [1500, 8000], 'Noise decay': [20, 150], Metal: [0, 0.3], Click: [0.2, 0.8], Drive: [0, 0.4] },
  };
  const SYNTH_ZONES = {
    None: {},
    Bass: { 'Shape A': [0.3, 1], Sub: [0.3, 1], Detune: [0, 15], Cutoff: [150, 1500], Resonance: [0.7, 8], 'Env amount': [0.5, 3], 'Filter decay': [50, 400], Attack: [1, 10], Decay: [100, 600], Sustain: [0.3, 0.9], Release: [20, 200], Vibrato: [0, 0.03], Drive: [0, 0.6] },
    Keys: { 'Shape A': [0, 0.6], Sub: [0, 0.3], Cutoff: [800, 6000], Resonance: [0.5, 4], 'Env amount': [0.3, 2], Attack: [1, 20], Decay: [300, 2000], Sustain: [0.2, 0.7], Release: [100, 800], Vibrato: [0, 0.08], Drive: [0, 0.3] },
    Lead: { 'Shape A': [0.4, 1], Detune: [5, 25], Cutoff: [1000, 8000], Resonance: [1, 8], Attack: [1, 60], Sustain: [0.6, 1], Release: [60, 400], Vibrato: [0.1, 0.4], 'LFO rate': [4, 7], Drive: [0, 0.6] },
    Pad: { Detune: [10, 40], Cutoff: [400, 4000], 'Env amount': [0, 1], Attack: [300, 1500], Decay: [500, 3000], Sustain: [0.6, 1], Release: [600, 3000], Vibrato: [0, 0.15], Sub: [0, 0.4], Drive: [0, 0.2] },
    Pluck: { Cutoff: [300, 2500], 'Env amount': [1.5, 4], 'Filter decay': [40, 300], Attack: [1, 5], Decay: [80, 500], Sustain: [0, 0.2], Release: [50, 300], Vibrato: [0, 0.05], Drive: [0, 0.4] },
  };
  const zoneCache = {};
  function zoneRanges(kind, zone) {
    const key = kind + ':' + zone;
    if (zoneCache[key]) return zoneCache[key];
    const list = kind === 'drum' ? DRUM : SYNTH, Z = (kind === 'drum' ? DRUM_ZONES : SYNTH_ZONES)[zone] || {};
    return (zoneCache[key] = list.map(s => Z[s.name] ? [toNorm(s, Z[s.name][0]), toNorm(s, Z[s.name][1])] : [0, 1]));
  }
  Object.assign(BL, { WAVES, DRUM, SYNTH, DRUM_ZONES, SYNTH_ZONES, toReal, toNorm, fmt });

  // ---------- sounds: fresh, mutate, cross
  function soundFresh(kind, zone) {
    const Z = zoneRanges(kind, zone);
    return { kind, zone, v: Z.map(([lo, hi]) => lerp(lo, hi, R())) };
  }
  function soundMutate(g, spread, locks) {
    const Z = zoneRanges(g.kind, g.zone);
    return { ...g, v: g.v.map((x, i) => {
      if (locks && locks[i]) return x;
      let y = fold01(x + gauss() * spread);
      const [lo, hi] = Z[i];
      if (y < lo) y = lerp(y, lo, 0.6); else if (y > hi) y = lerp(y, hi, 0.6);
      return y;
    }) };
  }
  function soundCross(a, b, locks) {
    return { ...a, v: a.v.map((x, i) => (locks && locks[i]) ? x : (R() < 0.5 ? x : (R() < 0.25 ? (x + b.v[i]) / 2 : b.v[i]))) };
  }
  Object.assign(BL, { soundFresh, soundMutate, soundCross });

  // ---------- the litter: four candidates, breed, cross, back, spread
  class Litter {
    constructor(ops, size = 4) { this.ops = ops; this.size = size; this.history = []; this.start = 0.25; this.refresh(true); }
    snapshot() { return clone({ items: this.items, parent: this.parent, hear: this.hear, spread: this.spread, streak: this.streak }); }
    push() { this.history.push(this.snapshot()); if (this.history.length > 40) this.history.shift(); }
    refresh(first) {
      if (!first) this.push();
      this.items = Array.from({ length: this.size }, () => this.ops.fresh());
      this.parent = -1; this.hear = 0; this.spread = this.start; this.streak = 0;
    }
    setSpread(s) { this.spread = clamp(s, 0.02, 0.5); }
    breed(i) {
      this.push();
      if (i === this.parent) { this.spread *= 0.6; this.streak = 0; }
      else { this.streak++; if (this.streak >= 2) this.spread *= 1.6; }
      this.setSpread(this.spread);
      const p = this.items[i];
      this.items = this.items.map((x, k) => (k === i ? p : this.ops.mutate(p, this.spread)));
      this.parent = i; this.hear = i;
    }
    cross(i) {
      const j = this.hear;
      if (i === j) return this.breed(i);
      this.push();
      const a = this.items[i], b = this.items[j];
      this.items = this.items.map((x, k) => (k === i || k === j) ? x : this.ops.mutate(this.ops.cross(a, b), this.spread * 0.5));
      this.parent = i;
    }
    back() {
      const s = this.history.pop();
      if (!s) return false;
      Object.assign(this, s); return true;
    }
    get heard() { return this.items[this.hear]; }
  }
  BL.Litter = Litter;

  // ---------- drum loops
  const STEPS = 16;
  const STYLES = {
    None: { kick: Array(16).fill(0.22), snare: Array(16).fill(0.18), hat: Array(16).fill(0.35), perc: Array(16).fill(0.12), swing: null, nudge: 0, bpm: null },
    'Boom bap': {
      kick: [.95, 0, .15, .1, 0, 0, .2, .15, .25, .5, .55, 0, 0, .15, .2, .1],
      snare: [0, 0, 0, 0, .97, 0, 0, .1, 0, 0, 0, .12, .97, 0, .08, .15],
      hat: [.9, .15, .8, .2, .9, .15, .8, .25, .9, .15, .8, .2, .9, .2, .8, .35],
      perc: [0, 0, .05, 0, 0, .12, 0, 0, 0, 0, .1, 0, 0, 0, 0, .2], swing: 0.6, nudge: 0.04, bpm: 90,
    },
    'Lo-fi': {
      kick: [.9, 0, .1, .2, 0, 0, .15, .3, .1, .2, .6, 0, 0, .2, .1, 0],
      snare: [0, 0, 0, .05, .9, 0, .05, .12, 0, .1, 0, .1, .9, 0, .1, .2],
      hat: [.75, .3, .6, .3, .75, .3, .6, .35, .75, .3, .6, .3, .75, .3, .6, .4],
      perc: [0, .1, 0, 0, 0, 0, .15, 0, 0, .1, 0, 0, .05, 0, .15, 0], swing: 0.64, nudge: 0.1, bpm: 78,
    },
    'Four floor': {
      kick: [.98, 0, 0, 0, .98, 0, 0, 0, .98, 0, 0, 0, .98, 0, 0, .12],
      snare: [0, 0, 0, 0, .9, 0, 0, 0, 0, 0, 0, 0, .9, 0, 0, .06],
      hat: [.1, .25, .95, .25, .1, .25, .95, .25, .1, .25, .95, .25, .1, .3, .95, .3],
      perc: [0, 0, .1, .25, 0, .15, 0, .2, 0, 0, .15, .25, 0, .2, 0, .1], swing: 0.52, nudge: 0.01, bpm: 122,
    },
    Broken: {
      kick: [.95, 0, .1, .3, 0, 0, .5, 0, 0, .6, .2, 0, .1, 0, .35, .1],
      snare: [0, 0, 0, 0, .95, 0, 0, .25, 0, .1, 0, .3, .9, 0, .3, .2],
      hat: [.8, .5, .7, .5, .8, .5, .7, .55, .8, .5, .7, .5, .8, .55, .7, .6],
      perc: [0, .1, 0, .2, 0, .15, 0, 0, .2, 0, .1, 0, 0, .25, 0, .15], swing: 0.55, nudge: 0.03, bpm: 104,
    },
  };
  const LANES = ['kick', 'snare', 'hat', 'perc'];
  const LANE_ZONE = ['Kick', 'Snare', 'Hats', 'Perc'];
  const oddsAt = (style, lane, pos) => STYLES[style][LANES[lane]][mod(Math.round(pos), STEPS)];

  function dedupe(hits) {
    hits.sort((a, b) => a.pos - b.pos);
    const out = [];
    for (const h of hits) { if (out.length && h.pos - out[out.length - 1].pos < 0.25) continue; out.push(h); }
    return out.slice(0, 32 * 8);
  }
  function drumBar(style, r) {
    const st = STYLES[style];
    return LANES.map(name => {
      const hits = [];
      st[name].forEach((p, s) => {
        if (r() < p) hits.push({ pos: mod(s + gaussOf(r) * st.nudge, STEPS), vel: clamp(0.42 + p * 0.55 + gaussOf(r) * 0.08, 0.08, 1) });
      });
      return hits;
    });
  }
  function drumFresh(style, bars) {
    const r = R, st = STYLES[style];
    const base = drumBar(style, r);
    const lanes = LANES.map(() => ({ hits: [] }));
    for (let b = 0; b < bars; b++) {
      base.forEach((hits, li) => {
        let bar = hits.map(h => ({ ...h }));
        if (b > 0 && r() < 0.35) bar = varyLane(bar, style, li, 1, r);
        if (bars > 1 && b === bars - 1 && (li === 1 || li === 3)) {
          for (let s = 12; s < 16; s++) if (r() < 0.35) bar.push({ pos: s + (r() < 0.3 ? 0.5 : 0), vel: 0.5 + r() * 0.4 });
        }
        bar.forEach(h => lanes[li].hits.push({ pos: h.pos + b * STEPS, vel: h.vel }));
      });
    }
    lanes.forEach(l => { l.hits = dedupe(l.hits); });
    return { style, len: bars * STEPS, swing: st.swing == null ? 0.5 + R() * 0.12 : st.swing, lanes };
  }
  function varyLane(hits, style, li, n, r) {
    for (let k = 0; k < n; k++) {
      const op = r();
      if (op < 0.4) {
        const ws = Array.from({ length: STEPS }, (_, s) => hits.some(h => Math.abs(h.pos - s) < 0.5) ? 0 : 0.05 + STYLES[style][LANES[li]][s]);
        const s = weighted(ws, r); if (s >= 0) hits.push({ pos: s, vel: 0.3 + r() * 0.6 });
      } else if (op < 0.7 && hits.length) {
        hits.splice(Math.floor(r() * hits.length), 1);
      } else if (hits.length) {
        const h = hits[Math.floor(r() * hits.length)];
        h.pos = mod(h.pos + (r() < 0.5 ? -1 : 1), STEPS);
      }
    }
    return hits;
  }
  // Every unlocked lane makes at least one real change; about 1 + spread * 8 on average.
  function drumMutate(g, spread, locks) {
    const c = clone(g);
    c.lanes.forEach((lane, li) => {
      if (locks && locks[li]) return;
      const before = JSON.stringify(lane.hits);
      let tries = 0;
      do {
        const n = 1 + poisson(spread * 8);
        for (let k = 0; k < n; k++) {
          const which = weighted([1, lane.hits.length ? 0.8 : 0, lane.hits.length ? 1.2 : 0]);
          if (which === 0) {
            const ws = Array.from({ length: c.len }, (_, s) => lane.hits.some(h => Math.abs(h.pos - s) < 0.5) ? 0 : 0.04 + oddsAt(g.style, li, s));
            const s = weighted(ws); if (s >= 0) lane.hits.push({ pos: s, vel: clamp(0.35 + oddsAt(g.style, li, s) * 0.6 + gauss() * 0.1, 0.08, 1) });
          } else if (which === 1) {
            const ws = lane.hits.map(h => 1.05 - oddsAt(g.style, li, h.pos));
            lane.hits.splice(weighted(ws), 1);
          } else {
            const h = lane.hits[Math.floor(R() * lane.hits.length)];
            let d = gauss() * spread * 0.6 * 8;
            if (Math.abs(d) < 0.5) d = (d < 0 ? -1 : 1) * (0.5 + R());
            if (R() < 0.7) d = Math.round(d) || (d < 0 ? -1 : 1);
            h.pos = mod(h.pos + d, c.len);
            if (R() < 0.4) h.vel = clamp(h.vel + gauss() * spread, 0.08, 1);
          }
        }
        lane.hits = dedupe(lane.hits);
      } while (JSON.stringify(lane.hits) === before && ++tries < 6);
    });
    c.swing = clamp(c.swing + gauss() * spread * 0.04, 0.5, 0.75);
    return c;
  }
  function drumCross(a, b, locks) {
    const c = clone(a);
    c.lanes = a.lanes.map((l, i) => clone((locks && locks[i]) || R() < 0.5 ? l : b.lanes[i]));
    if (b.len !== a.len) c.lanes.forEach(l => { l.hits = l.hits.filter(h => h.pos < c.len); });
    c.swing = R() < 0.5 ? a.swing : b.swing;
    return c;
  }
  function drumResize(g, bars) {
    const len = bars * STEPS, c = clone(g);
    c.lanes.forEach(l => {
      const out = [];
      if (len <= g.len) l.hits.forEach(h => { if (h.pos < len) out.push(h); });
      else for (let off = 0; off < len; off += g.len) l.hits.forEach(h => { if (h.pos + off < len) out.push({ ...h, pos: h.pos + off }); });
      l.hits = dedupe(out);
    });
    c.len = len;
    return c;
  }
  // Swing as a warp of time inside each eighth.
  function swingWarp(pos, sw) {
    const base = Math.floor(pos / 2) * 2, x = pos - base;
    return base + (x < 1 ? 2 * sw * x : 2 * sw + (2 - 2 * sw) * (x - 1));
  }
  Object.assign(BL, { STYLES, LANES, LANE_ZONE, drumFresh, drumMutate, drumCross, drumResize, swingWarp });

  // ---------- synth loops from eight traits
  const SCALES = {
    Minor: [0, 2, 3, 5, 7, 8, 10], Major: [0, 2, 4, 5, 7, 9, 11], Dorian: [0, 2, 3, 5, 7, 9, 10],
    Phrygian: [0, 1, 3, 5, 7, 8, 10], 'Minor pent': [0, 3, 5, 7, 10], 'Major pent': [0, 2, 4, 7, 9],
    Blues: [0, 3, 5, 6, 7, 10], Chromatic: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
  };
  const TRAITS = [
    ['Tonal', 'Atonal'], ['Mono', 'Poly'], ['Melodic', 'Rhythmic'], ['On beat', 'Off beat'],
    ['Even', 'Dynamic'], ['Clustered', 'Open'], ['Sparse', 'Dense'], ['Short', 'Long'],
  ];
  const NOTE_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
  const noteName = m => NOTE_NAMES[mod(m, 12)] + (Math.floor(m / 12) - 1);
  function foldInto(m, lo, hi) { while (m < lo) m += 12; while (m > hi) m -= 12; return m < lo ? lo : m; }
  // Scale degree to MIDI, relative to a root note.
  function degMidi(d, scale, root) { const sc = SCALES[scale], n = sc.length; return root + 12 * Math.floor(d / n) + sc[mod(d, n)]; }

  // cp: { bars, scale, lo, hi, prog } (pitches are drawn in C; the key is added when played)
  function drawSynth(tr, seed, cp) {
    const r = rng(seed);
    const [atonal, poly, rhythm, offb, dyn, open, dense, long] = tr;
    const sc = SCALES[cp.scale], n = sc.length, bars = cp.bars, mb = bars >= 8 ? 2 : 1;
    const root = foldInto(Math.round((cp.lo + (cp.hi - cp.lo) * 0.3) / 12) * 12, cp.lo, cp.hi);
    const span = n * (0.6 + open * 1.6);
    const lowDeg = -Math.round(span * 0.25), highDeg = Math.round(span * 0.75);
    // 1. a motif of onsets
    const motif = [];
    for (let bar = 0; bar < mb; bar++) {
      const count = clamp(Math.round(1 + dense * 14 + gaussOf(r) * 1.2), 1, 16);
      const ws = Array.from({ length: 16 }, (_, s) => s % 4 === 0 ? 1 + (1 - offb) * 5 : s % 2 === 0 ? 1 + (1 - offb) * 1.5 : 0.2 + offb * 2.6);
      if (bar === 0 && offb < 0.5) ws[0] += 6;
      for (let i = 0; i < count; i++) { const s = weighted(ws, r); if (s < 0) break; motif.push({ step: bar * 16 + s }); ws[s] = 0; }
    }
    motif.sort((a, b) => a.step - b.step);
    // 2. timing, length, velocity, pitch (as degrees, or semitones when atonal)
    let deg = 0;
    const pAtonal = clamp((atonal - 0.2) / 0.8, 0, 1);
    motif.forEach((m, i) => {
      const next = i + 1 < motif.length ? motif[i + 1].step : mb * 16 + motif[0].step;
      const gap = next - m.step;
      m.off = offb > 0.6 ? gaussOf(r) * (offb - 0.6) * 0.35 : 0;
      m.len = clamp(gap * (0.15 + long * 1.25), 0.5, 32);
      m.v = clamp(0.72 + gaussOf(r) * dyn * 0.2 + (m.step % 4 === 0 ? dyn * 0.2 : -dyn * 0.08), 0.08, 1);
      if (i > 0 && r() < rhythm * 0.8) { /* repeat the pitch */ }
      else if (i > 0) {
        const leap = r() < open * 0.55 ? 2 + Math.floor(r() * 4) : 1 + Math.floor(r() * 2);
        deg += (r() < 0.5 ? -1 : 1) * leap;
        if (deg < lowDeg) deg = lowDeg + (lowDeg - deg); if (deg > highDeg) deg = highDeg - (deg - highDeg);
        if (r() < 0.25 * (1 - atonal)) deg = Math.round(deg / n) * n; // drift home to the root
      }
      m.deg = deg;
      m.semi = r() < pAtonal ? Math.floor(r() * 18) - 6 : null;
      m.chord = [];
      if (poly > 0.15 && r() < poly * 0.9) {
        const extra = 1 + Math.floor(r() * poly * 4);
        const gapDeg = open < 0.33 ? (r() < 0.4 ? 1 : 2) : open < 0.66 ? 2 : (r() < 0.5 ? 4 : n);
        for (let k = 1; k <= extra; k++) m.chord.push(k * gapDeg + (pAtonal > 0.5 && r() < 0.4 ? 1 : 0));
      }
    });
    // 3. fill the loop, varying each repeat a little
    const notes = [];
    for (let b = 0; b < bars; b++) {
      const mbar = b % mb;
      const bRoot = cp.prog ? cp.prog[b % cp.prog.length] : 0;
      motif.filter(m => Math.floor(m.step / 16) === mbar).forEach(m => {
        let step = m.step - mbar * 16 + m.off, d = m.deg + bRoot, v = m.v, len = m.len;
        if (b >= mb && r() < 0.3) {
          const op = r();
          if (op < 0.25) return; // dropped
          if (op < 0.5) step = mod(step + (r() < 0.5 ? -1 : 1), 16);
          else if (op < 0.8) d += r() < 0.5 ? -1 : 1;
          else v = clamp(v + gaussOf(r) * 0.2, 0.08, 1);
        }
        const start = (b * 16 + step) / 4;
        const make = p => notes.push({ s: Math.max(0, start), l: len / 4, p: foldInto(p, cp.lo, cp.hi), v });
        const base = m.semi != null ? degMidi(bRoot, cp.scale, root) + m.semi : degMidi(d, cp.scale, root);
        make(base);
        m.chord.forEach(c => make(m.semi != null ? base + c * 2 : degMidi(d + c, cp.scale, root)));
      });
    }
    return tidyNotes(notes, tr, cp);
  }
  function tidyNotes(notes, tr, cp) {
    const L = cp.bars * 4;
    notes = notes.filter(x => x.s < L).sort((a, b) => a.s - b.s || a.p - b.p);
    const out = [];
    for (const x of notes) { if (out.some(o => o.p === x.p && Math.abs(o.s - x.s) < 1 / 16)) continue; out.push(x); }
    out.forEach(x => { x.l = clamp(Math.min(x.l, L - x.s), 1 / 16, L); x.p = foldInto(x.p, cp.lo, cp.hi); });
    if (tr[1] < 0.3) { // mono: no two notes at once
      const mono = [];
      for (const x of out) { const last = mono[mono.length - 1]; if (last && Math.abs(last.s - x.s) < 1e-6) continue; if (last && last.s + last.l > x.s) last.l = Math.max(1 / 16, x.s - last.s); mono.push(x); }
      return mono.slice(0, 256);
    }
    return out.slice(0, 256);
  }
  function scaleStep(p, k, scale, lo, hi) {
    const sc = SCALES[scale];
    let m = p;
    for (let i = 0; i < Math.abs(k); i++) { do { m += k > 0 ? 1 : -1; } while (!sc.includes(mod(m, 12))); }
    return foldInto(m, lo, hi);
  }
  function synthFresh(cp, bias) {
    const traits = TRAITS.map((_, i) => bias && bias[i] ? lerp(bias[i][0], bias[i][1], R()) : R());
    const seed = newSeed();
    return { traits, seed, bars: cp.bars, notes: drawSynth(traits, seed, cp) };
  }
  // Traits move first; then every bar makes at least one real change.
  function synthMutate(g, spread, cp, traitLocks) {
    const c = clone(g);
    c.traits = g.traits.map((t, i) => traitLocks && traitLocks[i] ? t : fold01(t + gauss() * spread));
    const bars = c.bars, atonal = c.traits[0];
    for (let b = 0; b < bars; b++) {
      const before = JSON.stringify(c.notes);
      let tries = 0;
      do {
        const n = 1 + poisson(spread * 6);
        for (let k = 0; k < n; k++) {
          const inBar = c.notes.filter(x => x.s >= b * 4 && x.s < b * 4 + 4);
          const op = weighted([inBar.length < 12 ? 0.8 + c.traits[6] : 0.1, inBar.length > 1 ? 1.4 - c.traits[6] : 0, inBar.length ? 1 : 0, inBar.length ? 1.2 : 0, inBar.length ? 0.6 : 0]);
          if (op === 0) {
            const src = inBar.length ? pick(inBar) : (c.notes.length ? pick(c.notes) : { p: Math.round((cp.lo + cp.hi) / 2), l: 0.5, v: 0.8 });
            const step = Math.floor(R() * 16) / 4;
            c.notes.push({ s: b * 4 + step, l: src.l, p: scaleStep(src.p, Math.round(gauss() * 2), atonal > 0.6 ? 'Chromatic' : cp.scale, cp.lo, cp.hi), v: src.v });
          } else if (op === 1) {
            c.notes.splice(c.notes.indexOf(pick(inBar)), 1);
          } else if (op === 2) {
            const x = pick(inBar);
            let d = gauss() * spread * 2;
            if (Math.abs(d) < 0.25) d = d < 0 ? -0.25 : 0.25;
            if (R() < 0.7) d = Math.round(d * 4) / 4 || 0.25;
            x.s = clamp(x.s + d, b * 4, b * 4 + 3.99);
          } else if (op === 3) {
            const x = pick(inBar);
            x.p = scaleStep(x.p, (R() < 0.5 ? -1 : 1) * (1 + Math.floor(R() * (1 + c.traits[5] * 3))), atonal > 0.6 ? 'Chromatic' : cp.scale, cp.lo, cp.hi);
          } else if (op === 4) {
            const x = pick(inBar);
            if (R() < 0.5) x.l = clamp(x.l * Math.pow(2, gauss()), 1 / 16, 8); else x.v = clamp(x.v + gauss() * 0.25, 0.08, 1);
          }
        }
        c.notes = tidyNotes(c.notes, c.traits, cp);
      } while (JSON.stringify(c.notes) === before && ++tries < 6);
    }
    return c;
  }
  function synthCross(a, b, cp, traitLocks) {
    const c = clone(a);
    c.traits = a.traits.map((t, i) => traitLocks && traitLocks[i] ? t : (R() < 0.3 ? (t + b.traits[i]) / 2 : R() < 0.5 ? t : b.traits[i]));
    c.notes = [];
    for (let bar = 0; bar < c.bars; bar++) {
      const from = R() < 0.5 ? a : b;
      from.notes.filter(x => x.s >= bar * 4 && x.s < bar * 4 + 4).forEach(x => c.notes.push({ ...x }));
    }
    c.notes = tidyNotes(c.notes, c.traits, cp);
    return c;
  }
  function synthResize(g, bars, cp) {
    const c = clone(g), L = bars * 4, oldL = g.bars * 4;
    const out = [];
    if (L <= oldL) g.notes.forEach(x => { if (x.s < L) out.push({ ...x }); });
    else for (let off = 0; off < L; off += oldL) g.notes.forEach(x => { if (x.s + off < L) out.push({ ...x, s: x.s + off }); });
    c.bars = bars; c.notes = tidyNotes(out, c.traits, { ...cp, bars });
    return c;
  }
  function synthRedraw(g, cp) { return { ...g, notes: drawSynth(g.traits, g.seed, { ...cp, bars: g.bars }) }; }
  Object.assign(BL, { SCALES, TRAITS, NOTE_NAMES, noteName, drawSynth, synthFresh, synthMutate, synthCross, synthResize, synthRedraw, scaleStep, foldInto });

  // ---------- effects chains
  const FX_TYPES = {
    drive: { name: 'Drive', params: ['Amount', 'Fold', 'Tone', 'Out'] },
    crush: { name: 'Crush', params: ['Bits', 'Tone', 'Mix', 'Out'] },
    filter: { name: 'Filter', params: ['Cutoff', 'Resonance', 'Mode', 'Sweep'] },
    comb: { name: 'Comb', params: ['Time', 'Feedback', 'Depth', 'Rate'] },
    delay: { name: 'Delay', params: ['Time', 'Feedback', 'Tone', 'Ping-pong'] },
    reverb: { name: 'Reverb', params: ['Size', 'Decay', 'Damping', 'Mix'] },
    gate: { name: 'Gate', params: ['Rate', 'Depth', 'Smooth', 'Out'] },
    ring: { name: 'Ring', params: ['Freq', 'Shape', 'Mix', 'Out'] },
    tape: { name: 'Tape', params: ['Wow', 'Wow rate', 'Flutter', 'Saturate'] },
    comp: { name: 'Comp', params: ['Threshold', 'Ratio', 'Attack', 'Release'] },
    width: { name: 'Width', params: ['Haas', 'Spin', 'Rate', 'Out'] },
  };
  const CHARACTERS = {
    None: Object.keys(FX_TYPES),
    Space: ['reverb', 'delay', 'width', 'comb'],
    Dirt: ['drive', 'crush', 'tape', 'comp'],
    Motion: ['filter', 'comb', 'gate', 'width'],
    Glitch: ['ring', 'gate', 'crush', 'comb'],
  };
  const DELAY_BEATS = [0.25, 0.5, 0.75, 1, 1.5, 2];
  const GATE_BEATS = [1, 0.5, 0.25, 0.125];
  function fxShow(fx) {
    const p = fx.p, bpm = T.bpm;
    const pct = x => Math.round(x * 100) + '%';
    const hz = v => v >= 1000 ? (v / 1000).toFixed(1) + ' kHz' : Math.round(v) + ' Hz';
    switch (fx.type) {
      case 'drive': return ['x' + (1 + p[0] * 24).toFixed(1), pct(p[1]), hz(600 * Math.pow(30, p[2])), pct(p[3])];
      case 'crush': return [(16 - p[0] * 14).toFixed(1) + ' bits', hz(500 * Math.pow(36, p[1])), pct(p[2]), pct(p[3])];
      case 'filter': return [hz(100 * Math.pow(100, p[0])), (0.5 + p[1] * 14).toFixed(1), ['low', 'band', 'high'][Math.min(2, Math.floor(p[2] * 3))], pct(p[3])];
      case 'comb': return [(0.2 * Math.pow(125, p[0])).toFixed(1) + ' ms', Math.round((p[1] * 2 - 1) * 90) + '%', pct(p[2]), (0.05 * Math.pow(100, p[3])).toFixed(2) + ' Hz'];
      case 'delay': return [['1/16', '1/8', '3/16', '1/4', '3/8', '1/2'][Math.min(5, Math.floor(p[0] * 6))], pct(p[1] * 0.85), hz(800 * Math.pow(15, p[2])), pct(p[3])];
      case 'reverb': return [(0.4 + p[0] * 3.6).toFixed(1) + ' s', pct(p[1]), pct(p[2]), pct(p[3])];
      case 'gate': return [['1/4', '1/8', '1/16', '1/32'][Math.min(3, Math.floor(p[0] * 4))], pct(p[1]), p[2] < 0.5 ? 'hard' : 'soft', pct(p[3])];
      case 'ring': return [hz(5 * Math.pow(600, p[0])), p[1] < 0.5 ? 'sine' : 'square', pct(p[2]), pct(p[3])];
      case 'tape': return [pct(p[0]), (0.2 * Math.pow(15, p[1])).toFixed(1) + ' Hz', pct(p[2]), pct(p[3])];
      case 'comp': return [Math.round(-50 + p[0] * 40) + ' dB', (2 + p[1] * 18).toFixed(1) + ':1', (1 + p[2] * 99).toFixed(0) + ' ms', (30 + p[3] * 470).toFixed(0) + ' ms'];
      case 'width': return [(p[0] * 25).toFixed(1) + ' ms', pct(p[1]), (0.1 * Math.pow(40, p[2])).toFixed(1) + ' Hz', pct(p[3])];
    }
    return p.map(pct);
  }
  function fxFresh(char) {
    const pool = CHARACTERS[char] || CHARACTERS.None;
    const n = 2 + Math.floor(R() * 3), fx = [];
    const bag = pool.slice();
    for (let i = 0; i < n; i++) {
      const type = bag.length ? bag.splice(Math.floor(R() * bag.length), 1)[0] : pick(pool);
      fx.push({ type, p: [R(), R(), R(), R()], lock: false });
    }
    // reverb and delay sound best at the end of the chain
    fx.sort((a, b) => (a.type === 'reverb') - (b.type === 'reverb'));
    return { char, mix: 0.35 + R() * 0.45, fx };
  }
  function fxMutate(g, spread, wild) {
    const c = clone(g), pool = CHARACTERS[c.char] || CHARACTERS.None;
    const jump = spread * (wild ? 1.6 : 0.5);
    c.fx.forEach(f => {
      if (f.lock) return;
      f.p = f.p.map(x => fold01(x + gauss() * spread));
      if (R() < jump * 0.5) f.type = pick(pool.filter(t => t !== f.type));
    });
    if (R() < jump * 0.4 && c.fx.length > 1) {
      const i = Math.floor(R() * (c.fx.length - 1));
      if (!c.fx[i].lock && !c.fx[i + 1].lock) [c.fx[i], c.fx[i + 1]] = [c.fx[i + 1], c.fx[i]];
    }
    if (R() < jump * 0.4) {
      if (c.fx.length < 5 && R() < 0.5) c.fx.splice(Math.floor(R() * (c.fx.length + 1)), 0, { type: pick(pool), p: [R(), R(), R(), R()], lock: false });
      else { const free = c.fx.filter(f => !f.lock); if (free.length && c.fx.length > 1) c.fx.splice(c.fx.indexOf(pick(free)), 1); }
    }
    if (wild && R() < 0.2) {
      const free = c.fx.map((f, i) => f.lock ? -1 : i).filter(i => i >= 0);
      const shuffled = free.map(i => c.fx[i]).sort(() => R() - 0.5);
      free.forEach((i, k) => { c.fx[i] = shuffled[k]; });
    }
    c.mix = clamp(c.mix + gauss() * spread * 0.5, 0.1, 1);
    return c;
  }
  function fxCross(a, b) {
    const c = clone(a);
    c.fx = a.fx.slice(0, Math.ceil(a.fx.length / 2)).concat(b.fx.slice(Math.floor(b.fx.length / 2))).slice(0, 5).map(clone);
    c.mix = (a.mix + b.mix) / 2;
    return c;
  }
  Object.assign(BL, { FX_TYPES, CHARACTERS, fxFresh, fxMutate, fxCross, fxShow });

  // ---------- audio
  let ctx = null, out = null, noiseBuf = null;
  const curveCache = new Map();
  function tanhCurve(amount) {
    const k = Math.round(amount * 20) / 20, key = 't' + k;
    if (curveCache.has(key)) return curveCache.get(key);
    const n = 1024, c = new Float32Array(n), g = 1 + k * 20;
    for (let i = 0; i < n; i++) { const x = i / (n - 1) * 2 - 1; c[i] = Math.tanh(x * g) / Math.tanh(g); }
    curveCache.set(key, c); return c;
  }
  function foldCurve(f) {
    const k = Math.round(f * 20) / 20, key = 'f' + k;
    if (curveCache.has(key)) return curveCache.get(key);
    const n = 2048, c = new Float32Array(n);
    for (let i = 0; i < n; i++) { const x = i / (n - 1) * 2 - 1; c[i] = lerp(Math.tanh(x * 2.5) / Math.tanh(2.5), Math.sin(x * Math.PI * (0.5 + k * 3)), k); }
    curveCache.set(key, c); return c;
  }
  function crushCurve(bits) {
    const k = Math.round(bits * 2) / 2, key = 'c' + k;
    if (curveCache.has(key)) return curveCache.get(key);
    const n = 4096, c = new Float32Array(n), steps = Math.pow(2, k - 1);
    for (let i = 0; i < n; i++) { const x = i / (n - 1) * 2 - 1; c[i] = Math.round(x * steps) / steps; }
    curveCache.set(key, c); return c;
  }
  const irCache = new Map();
  function impulse(size, decay, damp) {
    const key = [size, decay, damp].map(x => Math.round(x * 10)).join(':');
    if (irCache.has(key)) return irCache.get(key);
    const sr = ctx.sampleRate, len = Math.floor(sr * (0.4 + size * 3.6)), buf = ctx.createBuffer(2, len, sr);
    const pw = 1.5 + (1 - decay) * 5;
    for (let ch = 0; ch < 2; ch++) {
      const d = buf.getChannelData(ch); let lp = 0; const a = 0.15 + (1 - damp) * 0.8;
      for (let i = 0; i < len; i++) { lp += a * ((R() * 2 - 1) - lp); d[i] = lp * Math.pow(1 - i / len, pw); }
    }
    irCache.set(key, buf); return buf;
  }

  BL.audio = function () {
    if (!ctx) {
      const AC = global.AudioContext || global.webkitAudioContext;
      ctx = new AC({ latencyHint: 'interactive' });
      const comp = ctx.createDynamicsCompressor();
      comp.threshold.value = -12; comp.knee.value = 6; comp.ratio.value = 6; comp.attack.value = 0.003; comp.release.value = 0.15;
      const clip = ctx.createWaveShaper(); clip.curve = tanhCurve(0.05);
      out = ctx.createGain(); out.gain.value = 0.8;
      out.connect(comp); comp.connect(clip); clip.connect(ctx.destination);
      noiseBuf = ctx.createBuffer(1, ctx.sampleRate * 2, ctx.sampleRate);
      const d = noiseBuf.getChannelData(0); for (let i = 0; i < d.length; i++) d[i] = R() * 2 - 1;
      BL.ctx = ctx; BL.out = out;
    }
    if (ctx.state !== 'running') ctx.resume();
    return ctx;
  };
  const G = v => { const g = ctx.createGain(); g.gain.value = v; return g; };
  const BQ = (type, f, q) => { const b = ctx.createBiquadFilter(); b.type = type; b.frequency.value = f; if (q != null) b.Q.value = q; return b; };
  const OSC = (type, f, oscs) => { const o = ctx.createOscillator(); o.type = type; o.frequency.value = f; o.start(); oscs.push(o); return o; };
  const link = (...nodes) => { for (let i = 0; i < nodes.length - 1; i++) nodes[i].connect(nodes[i + 1]); };

  BL.playDrum = function (g, t, vel, dest) {
    if (!ctx || !g) return;
    const v = i => toReal(DRUM[i], g.v[i]);
    const pitch = v(0), sweep = v(1), sweepT = v(2) / 1000, decay = v(3) / 1000, noise = v(4), tone = v(5), nDecay = v(6) / 1000, metal = v(7), click = v(8), drive = v(9);
    t = Math.max(t, ctx.currentTime);
    const vg = G(vel * 0.9), sh = ctx.createWaveShaper(); sh.curve = tanhCurve(drive * 0.7);
    link(vg, sh, dest);
    const end = t + Math.max(decay, nDecay) * 1.3 + 0.05;
    if (noise < 0.98) {
      const o = ctx.createOscillator(), a = ctx.createGain();
      o.frequency.setValueAtTime(pitch * Math.pow(2, sweep / 12), t);
      o.frequency.exponentialRampToValueAtTime(pitch, t + sweepT);
      a.gain.setValueAtTime(0, t); a.gain.linearRampToValueAtTime(1 - noise * 0.85, t + 0.002); a.gain.setTargetAtTime(0, t + 0.002, decay / 5);
      link(o, a, vg); o.start(t); o.stop(end);
    }
    if (noise > 0.02) {
      const n = ctx.createBufferSource(), a = ctx.createGain();
      n.buffer = noiseBuf;
      const f = tone > 6000 ? BQ('highpass', tone * 0.6, 0.7) : BQ('bandpass', tone, 0.9);
      a.gain.setValueAtTime(0, t); a.gain.linearRampToValueAtTime(noise * (f.type === 'bandpass' ? 1.7 : 1), t + 0.001); a.gain.setTargetAtTime(0, t + 0.001, nDecay / 5);
      link(n, f, a, vg); n.start(t, R() * 1.5); n.stop(end);
    }
    if (metal > 0.03) {
      const base = 280 + pitch * 0.6, hp = BQ('highpass', 7000, 0.7), a = ctx.createGain();
      a.gain.setValueAtTime(0, t); a.gain.linearRampToValueAtTime(metal * 0.3, t + 0.001); a.gain.setTargetAtTime(0, t + 0.001, nDecay / 5);
      link(hp, a, vg);
      for (const k of [1, 1.4471, 1.617, 1.9265, 2.5028, 2.6637]) { const o = ctx.createOscillator(); o.type = 'square'; o.frequency.value = base * k; o.connect(hp); o.start(t); o.stop(end); }
    }
    if (click > 0.03) {
      const n = ctx.createBufferSource(), a = ctx.createGain();
      n.buffer = noiseBuf;
      a.gain.setValueAtTime(click * 0.9, t); a.gain.setTargetAtTime(0, t, 0.0015);
      link(n, BQ('highpass', 2500, 0.7), a, vg); n.start(t, R()); n.stop(t + 0.03);
    }
  };

  BL.playSynth = function (g, midi, t, dur, vel, dest) {
    if (!ctx || !g) return;
    const v = i => toReal(SYNTH[i], g.v[i]);
    const f0 = 440 * Math.pow(2, (midi - 69) / 12);
    const wa = WAVES[Math.min(3, Math.floor(g.v[0] * 4))], wb = WAVES[Math.min(3, Math.floor(g.v[1] * 4))];
    const mixB = v(2), tuneB = Math.round(v(3)), det = v(4), sub = v(5), cut = v(6), q = v(7), env = v(8), fd = v(9) / 1000;
    const at = v(10) / 1000, de = v(11) / 1000, su = v(12), re = v(13) / 1000, vib = v(14), lr = v(15), drv = v(16);
    t = Math.max(t, ctx.currentTime);
    const end = t + Math.max(dur, at), stopT = end + re * 1.6 + 0.05;
    const flt = BQ('lowpass', cut, q);
    flt.frequency.setValueAtTime(Math.min(18000, cut * Math.pow(2, env)), t);
    flt.frequency.setTargetAtTime(cut, t, fd / 3);
    const amp = ctx.createGain(), pk = vel * 0.2;
    amp.gain.setValueAtTime(0, t); amp.gain.linearRampToValueAtTime(pk, t + at);
    amp.gain.setTargetAtTime(pk * su, t + at, de / 4);
    amp.gain.setTargetAtTime(0, end, re / 5);
    const sh = ctx.createWaveShaper(); sh.curve = tanhCurve(drv * 0.5);
    link(flt, sh, amp, dest);
    const lfo = ctx.createOscillator(), lg = G(vib * 100);
    lfo.frequency.value = lr; lfo.connect(lg); lfo.start(t); lfo.stop(stopT);
    const mk = (type, freq, detune, gain) => {
      if (gain <= 0.01) return;
      const o = ctx.createOscillator(), gg = G(gain);
      o.type = type; o.frequency.value = freq; o.detune.value = detune; lg.connect(o.detune);
      link(o, gg, flt); o.start(t); o.stop(stopT);
    };
    mk(wa, f0, -det / 2, 1 - mixB * 0.7);
    mk(wb, f0 * Math.pow(2, tuneB / 12), det / 2, mixB);
    mk('sine', f0 / 2, 0, sub * 0.9);
  };

  function buildFx(fx, oscs) {
    const p = fx.p;
    switch (fx.type) {
      case 'drive': {
        const pre = G(1 + p[0] * 24), sh = ctx.createWaveShaper(), lp = BQ('lowpass', 600 * Math.pow(30, p[2]), 0.7), post = G(lerp(1, 0.4, p[0]) * lerp(0.5, 1.2, p[3]));
        sh.curve = foldCurve(p[1]); sh.oversample = '2x'; link(pre, sh, lp, post); return { in: pre, out: post };
      }
      case 'crush': {
        const i = G(1), o = G(lerp(0.5, 1.2, p[3])), dry = G(1 - p[2]), wet = G(p[2]), sh = ctx.createWaveShaper();
        sh.curve = crushCurve(16 - p[0] * 14);
        link(i, dry, o); link(i, sh, BQ('lowpass', 500 * Math.pow(36, p[1]), 0.7), wet, o); return { in: i, out: o };
      }
      case 'filter': {
        const f = BQ(['lowpass', 'bandpass', 'highpass'][Math.min(2, Math.floor(p[2] * 3))], 100 * Math.pow(100, p[0]), 0.5 + p[1] * 14);
        const o = G(f.type === 'bandpass' ? 1.6 : 1);
        if (p[3] > 0.05) { const l = OSC('sine', T.bpm / 240, oscs), lg = G(p[3] * 3000); link(l, lg); lg.connect(f.detune); }
        link(f, o); return { in: f, out: o };
      }
      case 'comb': {
        const i = G(1), o = G(0.8), time = 0.0002 * Math.pow(125, p[0]), d = ctx.createDelay(0.1), fb = G((p[1] * 2 - 1) * 0.88);
        d.delayTime.value = time;
        if (p[2] > 0.05) { const l = OSC('sine', 0.05 * Math.pow(100, p[3]), oscs), lg = G(time * 0.5 * p[2]); link(l, lg); lg.connect(d.delayTime); }
        link(i, o); link(i, d, fb, d); link(d, G(0.7), o); return { in: i, out: o };
      }
      case 'delay': {
        const i = G(1), o = G(1), beats = DELAY_BEATS[Math.min(5, Math.floor(p[0] * 6))], time = Math.min(1.9, beats * 60 / T.bpm);
        const d1 = ctx.createDelay(2), d2 = ctx.createDelay(2), lp = BQ('lowpass', 800 * Math.pow(15, p[2]), 0.5), fb = G(p[1] * 0.85);
        d1.delayTime.value = time; d2.delayTime.value = time;
        const pl = ctx.createStereoPanner(), pr = ctx.createStereoPanner(); pl.pan.value = -p[3]; pr.pan.value = p[3];
        const wet = G(0.55);
        link(i, o); link(i, d1, lp, fb, d2); link(d1, pl, wet); link(d2, pr, wet); link(d2, d1); link(wet, o);
        fb.gain.value = p[1] * 0.85; return { in: i, out: o };
      }
      case 'reverb': {
        const i = G(1), o = G(1), cv = ctx.createConvolver(), mix = 0.15 + p[3] * 0.75;
        cv.buffer = impulse(p[0], p[1], p[2]);
        link(i, G(1 - mix * 0.5), o); link(i, cv, G(mix * 1.4), o); return { in: i, out: o };
      }
      case 'gate': {
        const beats = GATE_BEATS[Math.min(3, Math.floor(p[0] * 4))], g = G(1 - p[1] / 2), o = G(lerp(0.6, 1.3, p[3]));
        const l = OSC(p[2] < 0.5 ? 'square' : 'sine', T.bpm / 60 / beats, oscs), lg = G(p[1] / 2);
        link(l, lg); lg.connect(g.gain); link(g, o); return { in: g, out: o };
      }
      case 'ring': {
        const i = G(1), o = G(lerp(0.6, 1.3, p[3])), rg = G(0), l = OSC(p[1] < 0.5 ? 'sine' : 'square', 5 * Math.pow(600, p[0]), oscs);
        l.connect(rg.gain); link(i, G(1 - p[2]), o); link(i, rg, G(p[2]), o); return { in: i, out: o };
      }
      case 'tape': {
        const d = ctx.createDelay(0.1), sh = ctx.createWaveShaper(), lp = BQ('lowpass', 9000, 0.5), o = G(lerp(1, 0.6, p[3]));
        d.delayTime.value = 0.012; sh.curve = tanhCurve(p[3] * 0.6);
        const w = OSC('sine', 0.2 * Math.pow(15, p[1]), oscs), wg = G(p[0] * 0.004); link(w, wg); wg.connect(d.delayTime);
        const fl = OSC('sine', 7 + p[2] * 5, oscs), fg = G(p[2] * 0.0005); link(fl, fg); fg.connect(d.delayTime);
        link(d, sh, lp, o); return { in: d, out: o };
      }
      case 'comp': {
        const c = ctx.createDynamicsCompressor(); const th = -50 + p[0] * 40, ratio = 2 + p[1] * 18;
        c.threshold.value = th; c.ratio.value = ratio; c.attack.value = (1 + p[2] * 99) / 1000; c.release.value = (30 + p[3] * 470) / 1000;
        const o = G(Math.min(4, Math.pow(10, (-th * (1 - 1 / ratio) * 0.45) / 20))); link(c, o); return { in: c, out: o };
      }
      case 'width': {
        const i = G(1); i.channelCount = 2; i.channelCountMode = 'explicit'; i.channelInterpretation = 'speakers';
        const sp = ctx.createChannelSplitter(2), mg = ctx.createChannelMerger(2), d = ctx.createDelay(0.05), pan = ctx.createStereoPanner(), o = G(lerp(0.6, 1.3, p[3]));
        d.delayTime.value = p[0] * 0.025;
        link(i, sp); sp.connect(mg, 0, 0); sp.connect(d, 1); d.connect(mg, 0, 1);
        const l = OSC('sine', 0.1 * Math.pow(40, p[2]), oscs), lg = G(p[1]); link(l, lg); lg.connect(pan.pan);
        link(mg, pan, o); return { in: i, out: o };
      }
    }
    const g = G(1); return { in: g, out: g };
  }
  function buildChain(chain, bypass) {
    const oscs = [], inp = G(1), outG = G(1);
    if (!chain || !chain.fx.length || bypass) { inp.connect(outG); return { in: inp, out: outG, oscs }; }
    const dry = G(1 - chain.mix), wet = G(chain.mix);
    link(inp, dry, outG);
    let node = inp;
    for (const fx of chain.fx) { const b = buildFx(fx, oscs); node.connect(b.in); node = b.out; }
    link(node, wet, outG);
    return { in: inp, out: outG, oscs };
  }

  // A track's bus: input, its effects chain (rebuilt with a short crossfade), volume, meter.
  class Bus {
    constructor(dest) {
      BL.audio();
      this.input = G(1); this.vol = G(0.8); this.mute = G(1);
      this.meter = ctx.createAnalyser(); this.meter.fftSize = 512; this.buf = new Float32Array(512);
      link(this.vol, this.mute, dest || out); this.mute.connect(this.meter);
      this.cur = null; this.chain = null; this.bypass = false;
      this.setFx(null);
    }
    setFx(chain, bypass) {
      if (chain !== undefined) this.chain = chain;
      if (bypass !== undefined) this.bypass = bypass;
      const old = this.cur, nw = buildChain(this.chain, this.bypass), t = ctx.currentTime;
      nw.fade = G(0); link(nw.out, nw.fade, this.vol); this.input.connect(nw.in);
      nw.fade.gain.setValueAtTime(0, t); nw.fade.gain.linearRampToValueAtTime(1, t + 0.04);
      if (old) {
        old.fade.gain.setValueAtTime(1, t); old.fade.gain.linearRampToValueAtTime(0, t + 0.04);
        setTimeout(() => { try { this.input.disconnect(old.in); } catch (e) { /* gone */ } old.fade.disconnect(); old.oscs.forEach(o => { try { o.stop(); } catch (e) { /* stopped */ } }); }, 150);
      }
      this.cur = nw;
    }
    setVolume(v) { this.vol.gain.setTargetAtTime(v, ctx.currentTime, 0.02); }
    setMuted(m) { this.mute.gain.setTargetAtTime(m ? 0 : 1, ctx.currentTime, 0.01); }
    level() { this.meter.getFloatTimeDomainData(this.buf); let p = 0; for (const x of this.buf) p = Math.max(p, Math.abs(x)); return p; }
  }
  BL.Bus = Bus;

  // ---------- transport: a look-ahead scheduler in beats
  const T = BL.transport = {
    bpm: 92, playing: false, t0: 0, next: 0, sources: new Set(), timer: null,
    beatAt(time) { return (time - T.t0) * T.bpm / 60; },
    timeOf(beat) { return T.t0 + beat * 60 / T.bpm; },
    now() { return T.playing && ctx ? T.beatAt(ctx.currentTime) : 0; },
    start() { BL.audio(); T.t0 = ctx.currentTime + 0.08; T.next = 0; T.playing = true; clearInterval(T.timer); T.timer = setInterval(T.tick, 25); T.tick(); },
    stop() { T.playing = false; clearInterval(T.timer); },
    setBpm(bpm) {
      bpm = clamp(Math.round(bpm), 50, 180);
      if (T.playing && ctx) { const b = T.beatAt(ctx.currentTime); T.bpm = bpm; T.t0 = ctx.currentTime - b * 60 / bpm; T.next = Math.max(Math.ceil(b * 4) / 4, T.next); } else T.bpm = bpm;
    },
    tick() {
      if (!T.playing) return;
      while (T.timeOf(T.next) < ctx.currentTime + 0.15) {
        const b0 = T.next, b1 = b0 + 0.25;
        for (const s of T.sources) { try { s(b0, b1); } catch (e) { console.error(e); } }
        T.next = b1;
      }
    },
    // The absolute beat at which a loop event at `pos` falls in [b0, b1), or -1.
    inWindow(pos, L, b0, b1) { let a = Math.floor((b0 - pos) / L) * L + pos; if (a < b0) a += L; return a < b1 ? a : -1; },
  };

  // ---------- drawing helpers shared by both pages
  BL.fitCanvas = function (cv) {
    const dpr = Math.min(2, global.devicePixelRatio || 1), w = cv.clientWidth, h = cv.clientHeight;
    if (cv.width !== Math.round(w * dpr) || cv.height !== Math.round(h * dpr)) { cv.width = Math.round(w * dpr); cv.height = Math.round(h * dpr); }
    const c = cv.getContext('2d'); c.setTransform(dpr, 0, 0, dpr, 0, 0); c.clearRect(0, 0, w, h);
    return { c, w, h };
  };
  BL.drawDrumLoop = function (cv, g, colors, opts = {}) {
    const { c, w, h } = BL.fitCanvas(cv);
    if (!g) return;
    const rows = 4, rh = h / rows;
    for (let s = 0; s < g.len; s += 4) { c.fillStyle = s % 16 === 0 ? colors.barLine : colors.beatLine; c.fillRect(Math.round(s / g.len * w), 0, 1, h); }
    if (opts.locks) opts.locks.forEach((l, i) => { if (l) { c.fillStyle = colors.lock; c.fillRect(0, h - (i + 1) * rh, w, rh); } });
    g.lanes.forEach((lane, li) => {
      const y0 = h - (li + 1) * rh;
      c.fillStyle = colors.lanes[li];
      lane.hits.forEach(hit => {
        const x = swingWarp(hit.pos, g.swing) / g.len * w, hh = Math.max(2, (rh - 2) * (0.3 + hit.vel * 0.7));
        c.fillRect(x, y0 + (rh - hh) / 2, Math.max(2, w / g.len * 0.6), hh);
      });
    });
  };
  BL.drawNotes = function (cv, notes, beats, lo, hi, color, opts = {}) {
    const { c, w, h } = BL.fitCanvas(cv);
    for (let b = 0; b < beats; b++) { c.fillStyle = b % 4 === 0 ? opts.barLine : opts.beatLine; if (c.fillStyle) c.fillRect(Math.round(b / beats * w), 0, 1, h); }
    if (opts.keyRows) {
      for (let m = lo; m <= hi; m++) if (opts.keyRows(m)) { c.fillStyle = opts.keyFill; c.fillRect(0, h - (m - lo + 1) / (hi - lo + 1) * h, w, h / (hi - lo + 1)); }
    }
    const nh = Math.max(2, h / (hi - lo + 1));
    notes.forEach(n => {
      c.globalAlpha = 0.45 + n.v * 0.55; c.fillStyle = color;
      const y = h - (n.p - lo + 1) / (hi - lo + 1) * h;
      c.fillRect(n.s / beats * w, y, Math.max(2, n.l / beats * w - 1), Math.max(2, nh - (nh > 4 ? 1 : 0)));
    });
    c.globalAlpha = 1;
  };

  global.BL = BL;
})(typeof window !== 'undefined' ? window : globalThis);
