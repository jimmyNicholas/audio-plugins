// Bloodline Track prototype (issue #51): behaviour through the one seam, Track actions and the offline render.
// Effects need Web Audio, so here chains pass audio through; the page runs them offline in the browser.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { createRequire } from 'node:module';
const require = createRequire(import.meta.url);
// The shared engine draws from Math.random, so seed that too (before the engine loads) to make every run the same.
let gen = null;
const builtin = Math.random;
Math.random = () => (gen ? gen() : builtin());
function mulberry32(a) { return function () { a |= 0; a = a + 0x6D2B79F5 | 0; let t = Math.imul(a ^ a >>> 15, 1 | a); t = t + Math.imul(t ^ t >>> 7, 61 | t) ^ t; return ((t ^ t >>> 14) >>> 0) / 4294967296; }; }
require('./bloodline-engine.js');
const T = require('./bloodline-track-core.js');

const peakOf = (l, r) => { let p = 0; for (let i = 0; i < l.length; i++) p = Math.max(p, Math.abs(l[i]), Math.abs(r[i])); return p; };
const db = x => 20 * Math.log10(x);
function track(n = 1, opts) { T.seed(n); gen = mulberry32(n * 7919 + 1); return T.createTrack(opts); }

test('stems add up to the mix when the limiter is idle', async () => {
  const t = track(1); t.lanes.forEach(l => { l.db = -12; });
  const res = await T.render(t);
  assert.ok(res.cut > 0.9999, 'limiter idle');
  let worst = 0;
  for (let i = 0; i < res.L; i++) { let l = 0, r = 0; res.stems.forEach(s => { l += s.l[i]; r += s.r[i]; }); worst = Math.max(worst, Math.abs(l - res.left[i]), Math.abs(r - res.right[i])); }
  assert.ok(worst < 1e-6, 'largest difference ' + worst);
  assert.equal(res.stems.length, t.lanes.length);
});

test('the limiter keeps the master at or under -1 dB', async () => {
  const t = track(2); t.lanes.forEach(l => { l.db = 12; }); t.master.db = 6;
  const res = await T.render(t);
  assert.ok(res.cut < 1, 'limiter working');
  assert.ok(db(peakOf(res.left, res.right)) <= -1 + 0.01, 'peak ' + db(peakOf(res.left, res.right)));
});

test('a loop is bars x 4 beats long at the Track tempo', async () => {
  const t = track(3); t.bpm = 100; T.setBars(t, 4);
  const res = await T.render(t);
  assert.equal(res.L, Math.round(4 * 4 * 60 / 100 * T.SR));
});

test('a muted lane is silent and solo leaves only the soloed lanes', async () => {
  const t = track(4); t.lanes.forEach(l => { l.db = -12; });
  T.toggle(t, t.lanes[1], 'mute');
  let res = await T.render(t);
  assert.equal(peakOf(res.stems[1].l, res.stems[1].r), 0);
  assert.ok(peakOf(res.stems[0].l, res.stems[0].r) > 0.01);
  T.toggle(t, t.lanes[1], 'mute'); T.toggle(t, t.lanes[2], 'solo');
  res = await T.render(t);
  assert.equal(peakOf(res.stems[0].l, res.stems[0].r), 0);
  assert.equal(peakOf(res.stems[1].l, res.stems[1].r), 0);
  assert.ok(peakOf(res.stems[2].l, res.stems[2].r) > 0.001);
});

test('Auto level leaves locked lanes alone and keeps the limiter idle', async () => {
  const t = track(5); t.lanes[1].db = -9; t.lanes[1].levelLocked = true; t.lanes[0].db = -30; t.lanes[2].db = 10;
  const n = T.autoLevel(t, await T.render(t));
  assert.equal(n, 2);
  assert.equal(t.lanes[1].db, -9);
  assert.notEqual(t.lanes[0].db, -30);
  const res = await T.render(t);
  assert.ok(db(res.prePeak) <= -1.4, 'mix peak before the limiter ' + db(res.prePeak));
  assert.equal(T.undo(t), 'auto level');
  assert.equal(t.lanes[0].db, -30);
});

test('a pad relabelled as Snare plays a snare part from the next New', () => {
  const t = track(6), lane = t.lanes[0], P = lane.pattern;
  const drum = P.chosen[3], before = lane.sound.pads[drum].v.slice();
  assert.equal(lane.sound.pads[drum].label, 'None');
  assert.ok(T.relabel(t, lane, drum, 'Snare'));
  assert.deepEqual(lane.sound.pads[drum].v, before, 'the sound is unchanged');
  assert.deepEqual(T.staleRows(lane), [false, false, false, true], 'the current loop is not rewritten yet');
  T.renew(t, lane, 'pattern');
  const lp = T.curLoop(lane);
  assert.equal(lp.roles[3], 'snare');
  for (let b = 0; b < lp.bars; b++) { assert.ok(lp.rows[3][b * 16 + 4] >= 0.45); assert.ok(lp.rows[3][b * 16 + 12] >= 0.45); }
  assert.deepEqual(T.staleRows(lane), [false, false, false, false]);
});

test('Rewrite row applies a new label now, unless the row is locked', () => {
  const t = track(7), lane = t.lanes[0], P = lane.pattern;
  T.relabel(t, lane, P.chosen[2], 'Kick');
  P.rowLocks[2] = true;
  assert.equal(T.rewriteRow(t, lane, 2), false);
  assert.equal(T.curLoop(lane).roles[2], 'hat');
  P.rowLocks[2] = false;
  assert.ok(T.rewriteRow(t, lane, 2));
  const lp = T.curLoop(lane);
  assert.equal(lp.roles[2], 'kick');
  assert.ok(lp.rows[2][0] >= 0.45, 'a kick on the one');
  assert.ok(T.back(t, lane, 'pattern'), 'Back undoes it');
  assert.equal(T.curLoop(lane).roles[2], 'hat');
});

test('two rows may share a label', () => {
  const t = track(8), lane = t.lanes[0];
  T.relabel(t, lane, lane.pattern.chosen[3], 'Hats');
  T.renew(t, lane, 'pattern');
  const lp = T.curLoop(lane);
  assert.equal(lp.roles[2], 'hat'); assert.equal(lp.roles[3], 'hat');
});

test('breeding a break works without the subdivision counts, and Back restores it', () => {
  const t = track(9), lane = t.lanes[0], before = JSON.stringify(lane.pattern.lit.items);
  T.breed(t, lane, 'pattern', 1);
  const items = lane.pattern.lit.items;
  assert.equal(lane.pattern.lit.hear, 1);
  assert.notEqual(JSON.stringify(items), before);
  items.forEach((lp, k) => { if (k !== 1) assert.notEqual(JSON.stringify(lp.rows), JSON.stringify(items[1].rows), 'child ' + k + ' changed'); });
  assert.ok(T.back(t, lane, 'pattern'));
  assert.equal(JSON.stringify(lane.pattern.lit.items), before);
});

test('locked rows survive Breed and New', () => {
  const t = track(10), lane = t.lanes[0];
  lane.pattern.rowLocks[1] = true;
  const kept = T.curLoop(lane).rows[1].join(',');
  T.breed(t, lane, 'pattern', 0); T.renew(t, lane, 'pattern');
  lane.pattern.lit.items.forEach(lp => assert.equal(lp.rows[1].join(','), kept));
});

test('the Dense trait makes busier breaks', () => {
  const count = dense => { let n = 0; for (let s = 1; s <= 12; s++) { const t = track(100 + s), lane = t.lanes[0], lp = T.curLoop(lane); lp.traits[3] = dense; T.redrawLoop(t, lane, lp); n += lp.rows.flat().filter(v => v >= 0.45).length; } return n; };
  assert.ok(count(0.95) > count(0.05) * 1.3);
});

test('adding a part: Cancel leaves the Track alone, Add to track appends a lane', () => {
  const t = track(11), n = t.lanes.length;
  T.addPart(t, 'synth'); T.cancelAdd(t);
  assert.equal(t.lanes.length, n); assert.equal(t.adding, null);
  const d = T.addPart(t, 'bass');
  assert.equal(t.adding.kind, 'bass');
  assert.equal(T.addToTrack(t), d);
  assert.equal(t.lanes.length, n + 1); assert.equal(t.lanes[n].kind, 'bass');
});

test('the draft plays in the Track, or alone', async () => {
  const t = track(12); t.lanes.forEach(l => { l.db = -12; });
  T.addPart(t, 'synth'); t.adding.draft.db = -12;
  let res = await T.render(t);
  assert.equal(res.stems.length, t.lanes.length + 1);
  assert.ok(res.stems.some(s => !s.draft && peakOf(s.l, s.r) > 0));
  t.draftAlone = true; res = await T.render(t);
  res.stems.forEach(s => { if (!s.draft) assert.equal(peakOf(s.l, s.r), 0); });
  assert.ok(peakOf(res.stems.at(-1).l, res.stems.at(-1).r) > 0);
});

test('Undo brings back a removed lane', () => {
  const t = track(13), lane = t.lanes[1];
  T.removeLane(t, lane);
  assert.ok(!t.lanes.some(l => l.id === lane.id));
  assert.equal(T.undo(t), 'remove');
  assert.ok(t.lanes.some(l => l.id === lane.id));
});

test('a bass following the kick puts most kicks under a note', () => {
  let hit = 0, all = 0;
  for (let s = 1; s <= 8; s++) {
    const t = track(200 + s), drums = t.lanes[0], bass = t.lanes[1];
    T.setFollow(t, bass, drums.id);
    T.renew(t, bass, 'pattern');
    const lp = T.curLoop(drums), kicks = []; lp.rows.forEach((row, r) => { if (lp.roles[r] === 'kick') row.forEach((v, k) => { if (v >= 0.45) kicks.push(k / 4); }); });
    const notes = T.curLoop(bass).notes;
    kicks.forEach(b => { all++; if (notes.some(n => Math.abs(n.s - b) < 0.13)) hit++; });
  }
  assert.ok(hit / all >= 0.7, hit + ' of ' + all);
});

test('swing applies to pitched parts on the same grid as drums, and a lane can opt out', async () => {
  const t = track(14, { dry: true }); const g = T.grooveNow(t); g.groove = 100;
  t.lanes = t.lanes.filter(l => l.kind === 'synth');
  const lane = t.lanes[0], lp = T.curLoop(lane); lp.notes = [{ s: 0.25, l: 0.2, p: 60, v: 1 }];
  lane.feel = 'on'; T.curVoice(lane)[10] = 0; // fastest attack
  const onset = res => { const l = res.stems[0].l; for (let i = 0; i < l.length; i++) if (Math.abs(l[i]) > 1e-4) return i / T.SR; return -1; };
  const sd = 60 / t.bpm / 4, swung = onset(await T.render(t));
  lane.followSwing = false; const straight = onset(await T.render(t));
  assert.ok(Math.abs(straight - sd) < 0.006, 'straight ' + straight);
  assert.ok(swung - straight > 0.02, 'swung later by ' + (swung - straight));
});

test('changing bars resizes every part', () => {
  const t = track(15); T.setBars(t, 4);
  t.lanes.forEach(l => { const lp = T.curLoop(l); assert.equal(lp.bars, 4); if (l.kind === 'drums') assert.equal(lp.rows[0].length, 64); });
});
