const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");
const {
  createEngine,
  createRouter,
  createSemitoneRouter,
  createTap,
  bendForCents,
  hertzForPitch,
} = require("../tap.js");

// Runs raw MIDI bytes through the whole device and returns what came out.
function setup() {
  const out = [];
  const tap = createTap((byte) => out.push(byte));
  const play = (bytes) => {
    out.length = 0;
    bytes.forEach(tap.input);
    return out.slice();
  };
  outputs.set(tap, out);
  return { tap, play };
}

// What the device sent while running fn, for changes made from the panel.
const outputs = new WeakMap();
function captured(tap, fn) {
  const out = outputs.get(tap);
  out.length = 0;
  fn();
  return out.slice();
}

// Splits output bytes into 3 byte channel messages. Fine for the notes, bends and controllers used here.
function messages(bytes) {
  const result = [];
  for (let i = 0; i < bytes.length; i += 3) result.push(bytes.slice(i, i + 3));
  return result;
}

const CENTRE = [0, 64]; // pitch bend 8192, low byte then high byte

// ---- Pitch helpers (from __tests__/utils/hertzHelpers and the parked engine tests) ----

test("concert pitch is A4 = 440 Hz in equal temperament", () => {
  assert.equal(hertzForPitch(69), 440);
  assert.ok(Math.abs(hertzForPitch(70) - 466.164) < 1e-3);
  assert.ok(Math.abs(hertzForPitch(81) - 880) < 1e-9);
  assert.ok(Math.abs(hertzForPitch(87) - 1244.508) < 1e-3);
  assert.ok(Math.abs(hertzForPitch(68) - 415.305) < 1e-3);
  assert.ok(Math.abs(hertzForPitch(57) - 220) < 1e-9);
  assert.ok(Math.abs(hertzForPitch(49) - 138.591) < 1e-3);
  assert.ok(Math.abs(hertzForPitch(60) - 261.626) < 1e-3); // middle C
  assert.ok(Math.abs(hertzForPitch(21) - 27.5) < 1e-9); // A0
  assert.ok(Math.abs(hertzForPitch(108) - 4186.009) < 1e-3); // C8
});

test("bend is centred for no detune and scaled to a 48 semitone range", () => {
  assert.equal(bendForCents(0), 8192);
  assert.equal(bendForCents(50), 8192 + 85); // 50 / 4800 * 8192 = 85.3
  assert.equal(bendForCents(-100), 8192 - 171);
  assert.equal(bendForCents(4800), 16383);
  assert.equal(bendForCents(-4800), 0);
});

// ---- Engine: pitch table, Swap, pedal (from swap.test.ts and the parked TapEngineTests) ----

test("every key starts at concert pitch", () => {
  const engine = createEngine();
  for (let note = 0; note < 128; note++) assert.equal(engine.pitchForNote(note), note);
});

test("the first release changes nothing", () => {
  const engine = createEngine();
  engine.noteReleased(69);
  assert.equal(engine.pitchForNote(69), 69);
});

test("Swap swaps the last two released keys", () => {
  const engine = createEngine();
  engine.noteReleased(69); // A4
  engine.noteReleased(71); // B4
  assert.equal(engine.pitchForNote(69), 71);
  assert.equal(engine.pitchForNote(71), 69);
});

test("releasing the same key twice does nothing", () => {
  const engine = createEngine();
  engine.noteReleased(69);
  engine.noteReleased(71);
  engine.noteReleased(71);
  assert.equal(engine.pitchForNote(69), 71);
  assert.equal(engine.pitchForNote(71), 69);
});

test("Swap moves pitches along a chain of releases", () => {
  const engine = createEngine();
  engine.noteReleased(60);
  engine.noteReleased(62); // 60 <-> 62
  engine.noteReleased(64); // 62 <-> 64
  assert.equal(engine.pitchForNote(60), 62);
  assert.equal(engine.pitchForNote(62), 64);
  assert.equal(engine.pitchForNote(64), 60);
});

test("reset puts every key back and forgets the last release", () => {
  const engine = createEngine();
  engine.noteReleased(69);
  engine.noteReleased(71);
  engine.reset();
  assert.equal(engine.pitchForNote(69), 69);
  assert.equal(engine.pitchForNote(71), 71);
  engine.noteReleased(60); // a first release again
  assert.equal(engine.pitchForNote(60), 60);
  assert.equal(engine.pitchForNote(71), 71);
});

test("out of range notes are ignored", () => {
  const engine = createEngine();
  engine.noteReleased(-1);
  engine.noteReleased(128);
  engine.noteReleased(60);
  assert.equal(engine.pitchForNote(60), 60); // still a first release
});

test("with the pedal down releases wait until it lifts, then apply in order", () => {
  const engine = createEngine();
  engine.setPedal(true);
  engine.noteReleased(60);
  engine.noteReleased(62);
  engine.noteReleased(64);
  assert.equal(engine.pitchForNote(60), 60);
  assert.equal(engine.pitchForNote(62), 62);
  engine.setPedal(false);
  assert.equal(engine.pitchForNote(60), 62);
  assert.equal(engine.pitchForNote(62), 64);
  assert.equal(engine.pitchForNote(64), 60);
});

test("pressing the pedal twice does not lose the queue", () => {
  const engine = createEngine();
  engine.setPedal(true);
  engine.noteReleased(60);
  engine.setPedal(true);
  engine.noteReleased(62);
  engine.setPedal(false);
  assert.equal(engine.pitchForNote(60), 62);
  engine.setPedal(false); // nothing left to apply
  assert.equal(engine.pitchForNote(60), 62);
});

// ---- Engine: Gravity, Move and settings (from __tests__/lib/data/modes) ----

const SWAP = 0;
const GRAVITY = 1;
const MOVE = 2;

function near(actual, expected, tolerance = 1e-9) {
  assert.ok(Math.abs(actual - expected) < tolerance, `${actual} is not close to ${expected}`);
}

function released(mode, mod, keys, setup = () => {}, random = undefined) {
  const engine = createEngine(undefined, random);
  engine.setMode(mode);
  engine.setMod(mod);
  setup(engine);
  keys.forEach((key) => engine.noteReleased(key));
  return engine;
}

function pitchesOf(engine, keys) {
  return keys.map((key) => engine.pitchForNote(key));
}

// Gravity's share of the gap at a Mod: 1% at Mod 1, three times as much per doubling.
function gravityShare(mod) {
  return 0.01 * Math.pow(Math.abs(mod), Math.log2(3));
}

test("the default mode is Swap, Mod 1 and bounds 0 to 127", () => {
  const engine = createEngine();
  assert.equal(engine.mode(), SWAP);
  assert.equal(engine.mod(), 1);
  assert.equal(engine.lowest(), 0);
  assert.equal(engine.highest(), 127);
});

test("Gravity pulls the last key toward the current key, in Hertz", () => {
  const engine = released(GRAVITY, 2, [57, 69]); // A3 220 Hz, A4 440 Hz
  near(hertzForPitch(engine.pitchForNote(57)), 220 + 220 * 0.03, 1e-6);
  assert.equal(engine.pitchForNote(69), 69); // the current key does not move
  const down = released(GRAVITY, 2, [69, 57]); // A4 pulled down toward A3
  near(hertzForPitch(down.pitchForNote(69)), 440 - 220 * 0.03, 1e-6);
});

test("Gravity does nothing on the first release or when the same key is released twice", () => {
  const engine = createEngine();
  engine.setMode(GRAVITY);
  engine.noteReleased(69);
  assert.equal(engine.pitchForNote(69), 69);
  engine.noteReleased(69);
  assert.equal(engine.pitchForNote(69), 69);
});

test("Move shifts the released key by Mod semitones, from the first release", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.noteReleased(69);
  assert.equal(engine.pitchForNote(69), 70); // move.test.ts: 440 Hz up one is 466.16 Hz
  near(hertzForPitch(engine.pitchForNote(69)), 466.16, 1e-2);
  engine.noteReleased(69);
  assert.equal(engine.pitchForNote(69), 71);
});

test("Move down shifts the released key down and leaves the last key alone", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setMod(-1);
  engine.noteReleased(69);
  engine.noteReleased(71);
  assert.equal(engine.pitchForNote(71), 70); // move.test.ts: 493.88 Hz down one is 466.16 Hz
  assert.equal(engine.pitchForNote(69), 68);
});

test("Move with Mod at 0 changes nothing", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setMod(0);
  engine.noteReleased(60);
  assert.equal(engine.pitchForNote(60), 60);
});

test("settings are kept in range", () => {
  const engine = createEngine();
  engine.setMod(-13);
  assert.equal(engine.mod(), -12);
  engine.setMod(12.5);
  assert.equal(engine.mod(), 12);
  engine.setMod(1.4);
  assert.equal(engine.mod(), 1.4);
  engine.setLowest(-5);
  assert.equal(engine.lowest(), 0);
  engine.setHighest(200);
  assert.equal(engine.highest(), 127);
  engine.setLowest(40.6);
  assert.equal(engine.lowest(), 41);
  engine.setMode(MOVE);
  assert.equal(engine.mode(), MOVE);
  engine.setMode(3);
  assert.equal(engine.mode(), MOVE);
  engine.setMode(-1);
  assert.equal(engine.mode(), SWAP);
});

test("settings that are not numbers are ignored", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setMode("bang");
  engine.setMod(NaN);
  engine.setLowest("bang");
  engine.setHighest(NaN);
  assert.equal(engine.mode(), MOVE);
  assert.equal(engine.mod(), 1);
  assert.equal(engine.lowest(), 0);
  assert.equal(engine.highest(), 127);
});

test("changing mode resets every key and forgets the last release", () => {
  const engine = createEngine();
  engine.noteReleased(69);
  engine.noteReleased(71);
  engine.setMode(MOVE);
  assert.equal(engine.pitchForNote(69), 69);
  assert.equal(engine.pitchForNote(71), 71);
  engine.setMode(GRAVITY);
  engine.noteReleased(60); // a first release again, so Gravity does nothing
  assert.equal(engine.pitchForNote(71), 71);
});

test("choosing the mode that is already on keeps the pitches", () => {
  const engine = createEngine();
  engine.noteReleased(69);
  engine.noteReleased(71);
  engine.setMode(SWAP); // say automation sends the same value again
  assert.equal(engine.pitchForNote(69), 71);
});

test("changing a setting does not reset the keys", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.noteReleased(60);
  engine.setMod(2);
  engine.setLowest(10);
  assert.equal(engine.pitchForNote(60), 61);
  engine.noteReleased(60);
  assert.equal(engine.pitchForNote(60), 63);
});

test("reset drops releases still waiting for the pedal", () => {
  const engine = createEngine();
  engine.setPedal(true);
  engine.noteReleased(60);
  engine.noteReleased(62);
  engine.reset();
  engine.setPedal(false);
  assert.equal(engine.pitchForNote(60), 60);
  assert.equal(engine.pitchForNote(62), 62);
});

test("the pedal queue uses the mode that is on when the pedal lifts", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setPedal(true);
  engine.noteReleased(60);
  engine.noteReleased(60);
  engine.setMod(2);
  engine.setPedal(false);
  assert.equal(engine.pitchForNote(60), 64);
});

// ---- Engine: Mod and bounds (ticket 06) ----

test("Move takes fractions of a semitone and moves up to an octave", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setMod(0.5);
  engine.noteReleased(60);
  near(engine.pitchForNote(60), 60.5);
  engine.setMod(-12);
  engine.noteReleased(62);
  assert.equal(engine.pitchForNote(62), 50);
});

test("Move stops at the Highest bound", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setHighest(60);
  engine.setMod(2);
  engine.noteReleased(57);
  assert.equal(engine.pitchForNote(57), 59);
  engine.noteReleased(57);
  assert.equal(engine.pitchForNote(57), 60);
  engine.noteReleased(57);
  assert.equal(engine.pitchForNote(57), 60);
});

test("Move stops at the Lowest bound", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setLowest(48);
  engine.setMod(-12);
  engine.noteReleased(55);
  assert.equal(engine.pitchForNote(55), 48);
});

test("Gravity stops at the bounds", () => {
  const low = released(GRAVITY, -12, [69, 81], (e) => e.setLowest(68)); // pushed down from A4
  assert.equal(low.pitchForNote(69), 68);
  const high = released(GRAVITY, 12, [57, 81], (e) => e.setHighest(60)); // pulled up from A3
  assert.equal(high.pitchForNote(57), 60);
});

test("bounds set the wrong way round are read the other way round", () => {
  const engine = createEngine();
  engine.setMode(MOVE);
  engine.setLowest(72);
  engine.setHighest(60);
  engine.setMod(12);
  engine.noteReleased(65);
  assert.equal(engine.pitchForNote(65), 72);
  engine.setMod(-12);
  engine.noteReleased(64);
  assert.equal(engine.pitchForNote(64), 60);
});

test("changing a bound moves no key until a mode changes it", () => {
  let changes = 0;
  const engine = createEngine(() => changes++);
  engine.setMode(MOVE);
  engine.setMod(12);
  engine.noteReleased(60); // 72
  const before = changes;
  engine.setHighest(64);
  assert.equal(engine.pitchForNote(60), 72);
  assert.equal(changes, before); // the drift view is not told
  engine.setMod(0);
  engine.noteReleased(60);
  assert.equal(engine.pitchForNote(60), 64);
});

test("Swap keeps both swapped pitches within the bounds", () => {
  const engine = createEngine();
  engine.setHighest(64);
  engine.noteReleased(60);
  engine.noteReleased(70); // 60 would take 70, above Highest
  assert.equal(engine.pitchForNote(60), 64);
  assert.equal(engine.pitchForNote(70), 60);
});

// ---- Engine: Swap passes pitches round the newest releases (ticket 11) ----

const C = 60;
const E = 64;
const G = 67;

// The Swap from before ticket 11: the last two released keys trade pitches.
function oldSwap(keys) {
  const pitches = Array.from({ length: 128 }, (_, note) => note);
  let last = -1;
  for (const key of keys) {
    if (last >= 0 && last !== key) [pitches[last], pitches[key]] = [pitches[key], pitches[last]];
    last = key;
  }
  return pitches;
}

test("Swap at Mod +1 or -1 matches the old Swap, release for release", () => {
  let seed = 7;
  const random = () => (seed = (seed * 16807) % 2147483647) / 2147483647;
  const keys = Array.from({ length: 300 }, () => 48 + Math.floor(random() * 12));
  for (const mod of [1, -1, 0.6, -1.4]) {
    const engine = createEngine();
    engine.setMod(mod);
    keys.forEach((key, i) => {
      engine.noteReleased(key);
      const expected = oldSwap(keys.slice(0, i + 1));
      for (let note = 48; note < 60; note++) assert.equal(engine.pitchForNote(note), expected[note], `Mod ${mod}`);
    });
  }
});

test("Swap at Mod +2 rotates the newest three releases forward", () => {
  // Mod 0 fills the list without changing anything, so the rotation starts from concert pitch.
  const engine = released(SWAP, 0, [C, E]);
  engine.setMod(2);
  engine.noteReleased(G);
  assert.deepEqual(pitchesOf(engine, [C, E, G]), [E, G, C]);
});

test("Swap at Mod -2 rotates the newest three releases the other way", () => {
  const engine = released(SWAP, 0, [C, E]);
  engine.setMod(-2);
  engine.noteReleased(G);
  assert.deepEqual(pitchesOf(engine, [C, E, G]), [G, C, E]);
});

test("Swap at Mod +2 from concert pitch swaps on the second release, then rotates three", () => {
  const engine = released(SWAP, 2, [C, E]);
  assert.deepEqual(pitchesOf(engine, [C, E]), [E, C]);
  engine.noteReleased(G); // C takes E's pitch (C), E takes G's, G takes C's (E)
  assert.deepEqual(pitchesOf(engine, [C, E, G]), [C, G, E]);
  const back = released(SWAP, -2, [C, E, G]); // C takes G's, E takes C's (E), G takes E's (C)
  assert.deepEqual(pitchesOf(back, [C, E, G]), [G, E, C]);
});

test("Swap rotates fewer keys than Mod asks for while the list fills", () => {
  const engine = released(SWAP, 12, [60]);
  assert.equal(engine.pitchForNote(60), 60); // the very first release
  engine.noteReleased(62);
  assert.deepEqual(pitchesOf(engine, [60, 62]), [62, 60]);
  engine.noteReleased(64);
  assert.deepEqual(pitchesOf(engine, [60, 62, 64]), [60, 64, 62]);
});

test("Swap rotates at most the newest 13 releases", () => {
  const keys = Array.from({ length: 15 }, (_, i) => 40 + i); // 40 to 54
  const engine = released(SWAP, 0, keys);
  engine.setMod(12);
  engine.noteReleased(55);
  for (let key = 40; key <= 42; key++) assert.equal(engine.pitchForNote(key), key); // left the list
  for (let key = 43; key <= 54; key++) assert.equal(engine.pitchForNote(key), key + 1);
  assert.equal(engine.pitchForNote(55), 43);
});

test("Swap with a Mod that rounds to 0 changes nothing", () => {
  for (const mod of [0, 0.4, -0.4]) {
    const engine = released(SWAP, mod, [60, 62, 64, 60]);
    assert.deepEqual(pitchesOf(engine, [60, 62, 64]), [60, 62, 64], `Mod ${mod}`);
  }
});

test("Swap: the same key twice in a row changes nothing", () => {
  const engine = released(SWAP, 2, [60, 62]);
  engine.noteReleased(62);
  assert.deepEqual(pitchesOf(engine, [60, 62]), [62, 60]);
});

test("Swap: a key released twice takes the pitch of its newest place", () => {
  const engine = released(SWAP, 0, [C, E]);
  engine.setMod(2);
  engine.noteReleased(C); // places C, E, C: E takes C's pitch, the newest C takes the old C's
  assert.deepEqual(pitchesOf(engine, [C, E]), [C, C]);
});

test("Swap with the pedal applies each release in turn when it lifts", () => {
  const engine = released(SWAP, 2, [], (e) => e.setPedal(true));
  [C, E, G].forEach((key) => engine.noteReleased(key));
  assert.deepEqual(pitchesOf(engine, [C, E, G]), [C, E, G]);
  engine.setPedal(false);
  assert.deepEqual(pitchesOf(engine, [C, E, G]), pitchesOf(released(SWAP, 2, [C, E, G]), [C, E, G]));
});

test("reset and a mode change clear Swap's list", () => {
  const afterReset = released(SWAP, 0, [C, E]);
  afterReset.reset();
  afterReset.setMod(2);
  afterReset.noteReleased(G); // a first release again
  assert.deepEqual(pitchesOf(afterReset, [C, E, G]), [C, E, G]);
  const afterMode = released(SWAP, 0, [C, E]);
  afterMode.setMode(MOVE);
  afterMode.setMode(SWAP);
  afterMode.setMod(2);
  afterMode.noteReleased(G);
  assert.deepEqual(pitchesOf(afterMode, [C, E, G]), [C, E, G]);
});

// ---- Engine: Gravity's curve (ticket 11) ----

test("Gravity's share of the gap triples each time Mod doubles", () => {
  const table = [
    [0.5, 0.3],
    [1, 1],
    [2, 3],
    [4, 9],
    [8, 27],
    [12, 51.3],
  ];
  for (const [mod, percent] of table) {
    const engine = released(GRAVITY, mod, [57, 69]); // A3 220 Hz, A4 440 Hz: a 220 Hz gap
    const share = (hertzForPitch(engine.pitchForNote(57)) - 220) / 220;
    near(share, gravityShare(mod), 1e-9);
    assert.equal(Math.round(share * 1000) / 10, percent, `Mod ${mod}`);
  }
});

test("Gravity with a negative Mod pushes the last key away on the same curve", () => {
  const down = released(GRAVITY, -4, [57, 69]);
  near(hertzForPitch(down.pitchForNote(57)), 220 - 220 * 0.09, 1e-6);
  const up = released(GRAVITY, -4, [69, 57]); // A4 pushed up, away from A3
  near(hertzForPitch(up.pitchForNote(69)), 440 + 220 * 0.09, 1e-6);
  assert.equal(up.pitchForNote(57), 57);
});

test("Gravity at Mod 0 changes nothing", () => {
  const engine = released(GRAVITY, 0, [57, 69]);
  assert.equal(engine.pitchForNote(57), 57);
});

test("a Gravity push below 0 Hz goes to Lowest", () => {
  // A3 220 Hz pushed away from A5 880 Hz by 51% of the 660 Hz gap would go below 0 Hz.
  assert.equal(released(GRAVITY, -12, [57, 81]).pitchForNote(57), 0);
  assert.equal(released(GRAVITY, -12, [57, 81], (e) => e.setLowest(24)).pitchForNote(57), 24);
});

test("Gravity never lands one key exactly on another", () => {
  const engine = released(GRAVITY, 12, []);
  for (let i = 0; i < 20; i++) {
    engine.noteReleased(i % 2 === 0 ? 60 : 72); // each release pulls the other key closer
    assert.ok(engine.pitchForNote(60) < engine.pitchForNote(72), `release ${i}`);
  }
});

// ---- Engine: Effect Range (ticket 12) ----

// A random function that returns each value in turn and counts its calls.
function sequence(...values) {
  const random = () => values[random.calls++ % values.length];
  random.calls = 0;
  return random;
}

function ranged(mode, mod, range, random, keys) {
  return released(mode, mod, keys, (engine) => engine.setRange(range), random);
}

const NEVER = () => {
  throw new Error("random was called");
};

test("Effect Range defaults to 0", () => {
  assert.equal(createEngine().range(), 0);
});

test("Effect Range 0 gives the same pitches as before and never calls the random function", () => {
  const keys = [60, 64, 67, 60, 72, 64];
  for (const mode of [SWAP, GRAVITY, MOVE]) {
    for (const mod of [-3, 1, 2.5]) {
      const before = released(mode, mod, keys);
      const after = ranged(mode, mod, 0, NEVER, keys);
      assert.deepEqual(pitchesOf(after, keys), pitchesOf(before, keys), `mode ${mode}, Mod ${mod}`);
    }
  }
});

test("Move at Mod 0, Effect Range 2 moves by anywhere from -1 to just under +1", () => {
  const engine = ranged(MOVE, 0, 2, sequence(0, 0.5, 0.999), []);
  engine.noteReleased(60);
  assert.equal(engine.pitchForNote(60), 59);
  engine.noteReleased(60);
  assert.equal(engine.pitchForNote(60), 59);
  engine.noteReleased(60);
  near(engine.pitchForNote(60), 59.998);
});

test("Effect Range is centred on Mod", () => {
  near(ranged(MOVE, 1, 1, sequence(0), [60]).pitchForNote(60), 60.5);
  near(ranged(MOVE, 1, 1, sequence(0.999), [60]).pitchForNote(60), 61.499);
});

test("Mod plus the random amount stops at ±12", () => {
  assert.equal(ranged(MOVE, 12, 24, sequence(0.999), [60]).pitchForNote(60), 72);
  assert.equal(ranged(MOVE, -12, 24, sequence(0), [60]).pitchForNote(60), 48);
  assert.equal(ranged(MOVE, 6, 24, sequence(0.999), [60]).pitchForNote(60), 72);
});

test("Gravity with a large Effect Range sometimes pushes", () => {
  // Mod 1, Effect Range 4: anywhere from -1 to just under 3.
  const push = ranged(GRAVITY, 1, 4, sequence(0), [57, 69]);
  near(hertzForPitch(push.pitchForNote(57)), 220 - 220 * gravityShare(1), 1e-6);
  const pull = ranged(GRAVITY, 1, 4, sequence(0.999), [57, 69]);
  near(hertzForPitch(pull.pitchForNote(57)), 220 + 220 * gravityShare(2.996), 1e-6);
});

test("Swap's count and direction change with the random amount", () => {
  const keys = [60, 62, 64, 65, 67];
  // Mod 2, Effect Range 2: Mod 1 to 3, so 2 to 4 keys.
  assert.deepEqual(pitchesOf(ranged(SWAP, 2, 2, sequence(0), keys), keys), pitchesOf(released(SWAP, 1, keys), keys));
  assert.deepEqual(pitchesOf(ranged(SWAP, 2, 2, sequence(0.999), keys), keys), pitchesOf(released(SWAP, 3, keys), keys));
  // Mod 0, Effect Range 4: a negative result turns the other way.
  assert.deepEqual(pitchesOf(ranged(SWAP, 0, 4, sequence(0), keys), keys), pitchesOf(released(SWAP, -2, keys), keys));
  // A result that rounds to 0 changes nothing.
  assert.deepEqual(pitchesOf(ranged(SWAP, 2, 4, sequence(0), keys), keys), keys);
});

test("the pedal picks one number per queued release, when it lifts", () => {
  const random = sequence(0, 0.5, 0.75);
  const engine = ranged(MOVE, 0, 2, random, []);
  engine.setPedal(true);
  [60, 62, 64].forEach((key) => engine.noteReleased(key));
  assert.equal(random.calls, 0);
  engine.setPedal(false);
  assert.equal(random.calls, 3);
  assert.deepEqual(pitchesOf(engine, [60, 62, 64]), [59, 62, 64.5]);
});

test("Effect Range is kept from 0 to 24, and anything but a number is ignored", () => {
  const engine = createEngine();
  engine.setRange(-1);
  assert.equal(engine.range(), 0);
  engine.setRange(30);
  assert.equal(engine.range(), 24);
  engine.setRange(2.37);
  assert.equal(engine.range(), 2.37);
  engine.setRange("bang");
  engine.setRange(NaN);
  assert.equal(engine.range(), 2.37);
});

test("setting Effect Range changes no pitch and does not tell the view", () => {
  let changes = 0;
  const engine = createEngine(() => changes++, sequence(0));
  engine.setMode(MOVE);
  engine.noteReleased(60);
  changes = 0;
  engine.setRange(4);
  assert.equal(engine.pitchForNote(60), 61);
  assert.equal(changes, 0);
});

// ---- Router: MPE channels (from plugin/tests/MpeRouterTests.cpp and the spike tests) ----

function routerSetup() {
  const out = [];
  const router = createRouter((byte) => out.push(byte));
  const run = (fn) => {
    out.length = 0;
    fn(router);
    return out.slice();
  };
  return { router, run };
}

test("a note gets channel 2 with its bend sent first", () => {
  const { run } = routerSetup();
  const bend = bendForCents(30);
  assert.deepEqual(
    run((r) => r.noteOn(60, 60.3, 100)),
    [0xe1, bend & 0x7f, bend >> 7, 0x91, 60, 100],
  );
});

test("the nearest whole note is sent and the bend covers the rest", () => {
  const { run } = routerSetup();
  const bend = bendForCents(-30);
  assert.deepEqual(
    run((r) => r.noteOn(60, 71.7, 100)),
    [0xe1, bend & 0x7f, bend >> 7, 0x91, 72, 100],
  );
});

test("note off is sent for the note that was actually played", () => {
  const { run } = routerSetup();
  run((r) => r.noteOn(69, 71, 100));
  assert.deepEqual(run((r) => r.noteOff(69, 40)), [0x81, 71, 40]);
});

test("a chord uses separate channels and note off finds the right one", () => {
  const { router, run } = routerSetup();
  run((r) => r.noteOn(60, 60, 100));
  run((r) => r.noteOn(64, 64, 100));
  assert.deepEqual(run((r) => r.noteOff(64, 0)), [0x82, 64, 0]);
  assert.equal(router.activeNotes(), 1);
});

test("two keys can sound at the same pitch on different channels", () => {
  const { run } = routerSetup();
  assert.deepEqual(run((r) => r.noteOn(60, 64, 100)).slice(3), [0x91, 64, 100]);
  assert.deepEqual(run((r) => r.noteOn(64, 64, 100)).slice(3), [0x92, 64, 100]);
  assert.deepEqual(run((r) => r.noteOff(64, 0)), [0x82, 64, 0]);
  assert.deepEqual(run((r) => r.noteOff(60, 0)), [0x81, 64, 0]);
});

test("a new note takes the channel that has been free longest", () => {
  const { run } = routerSetup();
  run((r) => r.noteOn(60, 60, 100)); // ch 2
  run((r) => r.noteOff(60, 0));
  // Channel 2 still has a release tail. Channels 3 to 16 have never been used.
  assert.equal(run((r) => r.noteOn(62, 62, 100))[3], 0x92);
});

test("with all 15 channels busy the oldest note is stopped", () => {
  const { router, run } = routerSetup();
  for (let i = 0; i < 15; i++) run((r) => r.noteOn(40 + i, 40 + i, 100));
  assert.equal(router.activeNotes(), 15);
  assert.deepEqual(run((r) => r.noteOn(80, 80, 100)), [0x81, 40, 0, 0xe1, ...CENTRE, 0x91, 80, 100]);
  assert.deepEqual(run((r) => r.noteOff(40, 0)), []); // already stopped
});

test("poly aftertouch follows its key to its channel and played note", () => {
  const { run } = routerSetup();
  run((r) => r.noteOn(60, 62, 100));
  assert.deepEqual(run((r) => r.aftertouch(60, 30)), [0xa1, 62, 30]);
  assert.deepEqual(run((r) => r.aftertouch(61, 30)), []);
});

test("all notes off stops every held note", () => {
  const { router, run } = routerSetup();
  run((r) => r.noteOn(60, 60, 100));
  run((r) => r.noteOn(64, 65, 100));
  assert.deepEqual(run((r) => r.allNotesOff()), [0x81, 60, 0, 0x82, 65, 0]);
  assert.equal(router.activeNotes(), 0);
});

// ---- Semitone router: plain MIDI on channel 1 ----

function semitoneSetup() {
  const out = [];
  const router = createSemitoneRouter((byte) => out.push(byte));
  const run = (fn) => {
    out.length = 0;
    fn(router);
    return out.slice();
  };
  return { router, run };
}

test("semitone output rounds each pitch to the nearest note on channel 1, with no bend", () => {
  const { run } = semitoneSetup();
  assert.deepEqual(run((r) => r.noteOn(60, 60.3, 100)), [0x90, 60, 100]);
  assert.deepEqual(run((r) => r.noteOn(62, 71.7, 90)), [0x90, 72, 90]);
  assert.deepEqual(run((r) => r.noteOn(64, 64.5, 80)), [0x90, 65, 80]);
  assert.deepEqual(run((r) => r.noteOff(62, 40)), [0x80, 72, 40]);
});

test("semitone output keeps notes in the MIDI range", () => {
  const { run } = semitoneSetup();
  assert.deepEqual(run((r) => r.noteOn(0, -0.4, 100)), [0x90, 0, 100]);
  assert.deepEqual(run((r) => r.noteOn(127, 130, 100)), [0x90, 127, 100]);
});

test("two keys on one note: the note off waits for the last of them", () => {
  const { router, run } = semitoneSetup();
  assert.deepEqual(run((r) => r.noteOn(60, 64.2, 100)), [0x90, 64, 100]);
  assert.deepEqual(run((r) => r.noteOn(64, 64, 90)), [0x80, 64, 0, 0x90, 64, 90]); // restarted
  assert.equal(router.activeNotes(), 2);
  assert.deepEqual(run((r) => r.noteOff(60, 0)), []); // 64 still held
  assert.deepEqual(run((r) => r.aftertouch(64, 30)), [0xa0, 64, 30]);
  assert.deepEqual(run((r) => r.noteOff(64, 20)), [0x80, 64, 20]);
  assert.equal(router.activeNotes(), 0);
  assert.equal(run((r) => r.noteOff(64, 0)).length, 0); // and no second note off
});

test("semitone all notes off stops each sounding note once", () => {
  const { router, run } = semitoneSetup();
  run((r) => r.noteOn(60, 60, 100));
  run((r) => r.noteOn(61, 60.4, 100)); // restarts 60
  run((r) => r.noteOn(64, 65, 100));
  assert.deepEqual(run((r) => r.allNotesOff()), [0x80, 60, 0, 0x80, 65, 0]);
  assert.equal(router.activeNotes(), 0);
});

// ---- Whole device: MIDI bytes in, MPE bytes out ----

test("A then B, release both: A now sounds at B's pitch", () => {
  const { play } = setup();
  play([0x90, 69, 100, 0x80, 69, 0]);
  play([0x90, 71, 100, 0x80, 71, 0]);
  const out = play([0x90, 69, 100]);
  assert.deepEqual(out.slice(3), [0x90 | 3, 71, 100]); // channel 4: 2 and 3 have release tails
  assert.deepEqual(out.slice(1, 3), CENTRE);
  assert.deepEqual(play([0x80, 69, 0]), [0x83, 71, 0]);
});

test("a held key keeps its pitch while other keys are released", () => {
  const { play } = setup();
  play([0x90, 60, 100]); // hold C
  play([0x90, 62, 100, 0x80, 62, 0]); // first release
  play([0x90, 64, 100, 0x80, 64, 0]); // 62 <-> 64
  assert.deepEqual(play([0x80, 60, 0]), [0x81, 60, 0]);
});

test("velocity zero note on counts as a release", () => {
  const { play } = setup();
  play([0x90, 69, 100, 0x90, 69, 0]);
  play([0x90, 71, 100, 0x90, 71, 0]);
  assert.equal(play([0x90, 69, 100])[4], 71);
});

test("input channel is ignored", () => {
  const { play } = setup();
  assert.deepEqual(play([0x95, 60, 100]).slice(3), [0x91, 60, 100]);
  assert.deepEqual(play([0x85, 60, 0]), [0x81, 60, 0]);
});

test("a note off with no matching note on changes nothing", () => {
  const { play } = setup();
  play([0x80, 69, 0]); // stray, for example the device loaded while a key was down
  play([0x90, 71, 100, 0x80, 71, 0]); // so this is the first release
  play([0x90, 72, 100, 0x80, 72, 0]); // 71 <-> 72
  assert.equal(play([0x90, 69, 100])[4], 69);
  assert.equal(play([0x90, 71, 100])[4], 72);
});

test("the same key played again while held is stopped first, without a release", () => {
  const { play } = setup();
  play([0x90, 60, 100]);
  assert.deepEqual(play([0x90, 60, 90]), [0x81, 60, 0, 0xe2, ...CENTRE, 0x92, 60, 90]);
  assert.deepEqual(play([0x80, 60, 0]), [0x82, 60, 0]);
  play([0x90, 62, 100, 0x80, 62, 0]);
  // 60 was released once, then 62: one swap.
  assert.equal(play([0x90, 60, 100])[4], 62);
});

test("a stolen note still counts as released when its key comes up", () => {
  const { play } = setup();
  for (let i = 0; i < 16; i++) play([0x90, 40 + i, 100]); // 40 is stolen by 55
  assert.deepEqual(play([0x80, 40, 0]), []);
  play([0x80, 55, 0]); // 40 <-> 55
  assert.equal(play([0x90, 40, 100])[4], 55);
});

test("pedal: nothing retunes until it lifts, and the pedal reaches the instrument", () => {
  const { play } = setup();
  assert.deepEqual(play([0xb0, 64, 127]), [0xb0, 64, 127]);
  play([0x90, 69, 100, 0x80, 69, 0]);
  play([0x90, 71, 100, 0x80, 71, 0]);
  assert.equal(play([0x90, 69, 100, 0x80, 69, 0])[4], 69);
  assert.deepEqual(play([0xb0, 64, 0]), [0xb0, 64, 0]);
  // Released in order 69, 71, 69: swap, then swap back.
  assert.equal(play([0x90, 69, 100])[4], 69);

  const second = setup();
  second.play([0xb0, 64, 127, 0x90, 69, 100, 0x80, 69, 0, 0x90, 71, 100, 0x80, 71, 0, 0xb0, 64, 0]);
  assert.equal(second.play([0x90, 69, 100])[4], 71);
});

test("pedal values from 64 count as down", () => {
  const { play } = setup();
  play([0xb0, 64, 64]);
  play([0x90, 69, 100, 0x80, 69, 0]);
  play([0x90, 71, 100, 0x80, 71, 0]);
  assert.equal(play([0x90, 69, 100])[4], 69); // held, so not queued
  play([0xb0, 64, 63]);
  assert.equal(play([0x90, 71, 100])[4], 69); // the lift applied 69 <-> 71
});

test("all notes off stops held notes and passes through", () => {
  const { play } = setup();
  play([0x90, 60, 100]);
  assert.deepEqual(play([0xb2, 123, 0]), [0x81, 60, 0, 0xb0, 123, 0]);
  assert.deepEqual(play([0x80, 60, 0]), []); // already stopped, and not a release
  play([0x90, 62, 100, 0x80, 62, 0]);
  assert.equal(play([0x90, 60, 100])[4], 60);
});

test("shared messages go to channel 1", () => {
  const { play } = setup();
  assert.deepEqual(play([0xb3, 1, 90]), [0xb0, 1, 90]); // mod wheel
  assert.deepEqual(play([0xe3, 0, 80]), [0xe0, 0, 80]); // pitch wheel
  assert.deepEqual(play([0xc3, 5]), [0xc0, 5]); // program change
  assert.deepEqual(play([0xd3, 40]), [0xd0, 40]); // channel pressure
});

test("running status is understood", () => {
  const { play } = setup();
  const out = play([0x90, 60, 100, 62, 100]);
  assert.deepEqual(messages(out).map((m) => m[0]), [0xe1, 0x91, 0xe2, 0x92]);
});

test("real time and sysex pass through", () => {
  const { play } = setup();
  assert.deepEqual(play([0xf8]), [0xf8]);
  assert.deepEqual(play([0xf0, 1, 2, 0xf7]), [0xf0, 1, 2, 0xf7]);
});

test("the panel's mode, settings and reset reach the engine", () => {
  const { tap, play } = setup();
  tap.setMode(MOVE);
  tap.setMod(2);
  play([0x90, 60, 100, 0x80, 60, 0]); // C4 moves up 2
  assert.deepEqual(messages(play([0x90, 60, 100])).at(-1).slice(1), [62, 100]);
  play([0x80, 60, 0]); // up 2 again
  tap.reset();
  assert.deepEqual(messages(play([0x90, 60, 100])).at(-1).slice(1), [60, 100]);
  play([0x80, 60, 0]);
  tap.setMode(SWAP);
  tap.setMod(1);
  play([0x90, 60, 100, 0x80, 60, 0, 0x90, 67, 100, 0x80, 67, 0]);
  assert.deepEqual(messages(play([0x90, 60, 100])).at(-1).slice(1), [67, 100]);
});

test("changing mode leaves held notes sounding and they still stop", () => {
  const { tap, play } = setup();
  play([0x90, 60, 100]);
  tap.setMode(MOVE);
  assert.deepEqual(play([0x80, 60, 0]), [0x81, 60, 0]);
  assert.equal(tap.activeNotes(), 0);
});

test("no stuck notes after fast playing with more than 15 keys held", () => {
  const { tap, play } = setup();
  let seed = 1;
  const random = () => (seed = (seed * 16807) % 2147483647) / 2147483647;
  const held = new Set();
  const sounding = new Map(); // "channel:note" -> count
  const track = (out) => {
    for (const [status, note] of messages(out)) {
      const key = `${status & 0x0f}:${note}`;
      if ((status & 0xf0) === 0x90) sounding.set(key, (sounding.get(key) || 0) + 1);
      if ((status & 0xf0) === 0x80) {
        assert.ok(sounding.get(key) > 0, `note off for ${key} that is not sounding`);
        sounding.set(key, sounding.get(key) - 1);
      }
    }
  };
  for (let i = 0; i < 2000; i++) {
    const note = 36 + Math.floor(random() * 30);
    if (held.has(note) && random() < 0.6) {
      held.delete(note);
      track(play([0x80, note, 0]));
    } else {
      held.add(note);
      track(play([0x90, note, 100]));
    }
    if (i % 97 === 0) track(play([0xb0, 64, random() < 0.5 ? 127 : 0]));
  }
  for (const note of held) track(play([0x80, note, 0]));
  assert.equal(tap.activeNotes(), 0);
  for (const [key, count] of sounding) assert.equal(count, 0, `${key} is stuck`);
});

// ---- Whole device with Output on Semitone ----

const MPE = 0;
const SEMITONE = 1;

function semitoneDevice() {
  const device = setup();
  device.tap.setOutput(SEMITONE);
  return device;
}

test("Semitone output: Swap stays exact on channel 1", () => {
  const { play } = semitoneDevice();
  play([0x90, 69, 100, 0x80, 69, 0]);
  play([0x90, 71, 100, 0x80, 71, 0]);
  assert.deepEqual(play([0x90, 69, 100]), [0x90, 71, 100]);
  assert.deepEqual(play([0x80, 69, 0]), [0x80, 71, 0]);
});

test("Semitone output: Move stays exact", () => {
  const { tap, play } = semitoneDevice();
  tap.setMode(MOVE);
  tap.setMod(-2);
  play([0x90, 60, 100, 0x80, 60, 0]);
  assert.deepEqual(play([0x90, 60, 100]), [0x90, 58, 100]);
});

test("Semitone output: Gravity is rounded to the nearest semitone", () => {
  const { tap, play } = semitoneDevice();
  tap.setMode(GRAVITY);
  tap.setMod(12);
  play([0x90, 57, 100, 0x80, 57, 0]); // A3, 220 Hz
  play([0x90, 69, 100, 0x80, 69, 0]); // A4, 440 Hz: A3 goes to 333 Hz, note 64.17
  assert.deepEqual(play([0x90, 57, 100]), [0x90, 64, 100]);
});

test("Semitone output: two keys tuned to one note do not stick", () => {
  const { tap, play } = semitoneDevice();
  tap.setMode(MOVE);
  tap.setMod(4);
  play([0x90, 60, 100, 0x80, 60, 0]); // key 60 moves onto E
  assert.deepEqual(play([0x90, 60, 100]), [0x90, 64, 100]);
  assert.deepEqual(play([0x90, 64, 100]), [0x80, 64, 0, 0x90, 64, 100]); // E restarts
  assert.deepEqual(play([0x80, 60, 0]), []); // E is still held by key 64
  assert.deepEqual(play([0x80, 64, 0]), [0x80, 64, 0]);
  assert.equal(tap.activeNotes(), 0);
});

test("Semitone output: controllers and the pedal still pass through on channel 1", () => {
  const { play } = semitoneDevice();
  assert.deepEqual(play([0xb3, 64, 127]), [0xb0, 64, 127]);
  assert.deepEqual(play([0xe3, 0, 80]), [0xe0, 0, 80]);
});

test("changing Output stops held notes, and their keys still count as released", () => {
  const { tap, play } = setup();
  play([0x90, 60, 100, 0x90, 62, 100]); // channels 2 and 3
  assert.deepEqual(
    captured(tap, () => tap.setOutput(SEMITONE)),
    [0x81, 60, 0, 0x82, 62, 0],
  );
  assert.equal(tap.activeNotes(), 0);
  assert.deepEqual(play([0x80, 60, 0, 0x80, 62, 0]), []); // already stopped
  assert.deepEqual(play([0x90, 60, 100]), [0x90, 62, 100]); // 60 <-> 62
  assert.deepEqual(captured(tap, () => tap.setOutput(MPE)), [0x80, 62, 0]);
  assert.deepEqual(play([0x80, 60, 0]), []);
});

test("Output settings that are not numbers or are the same again change nothing", () => {
  const { tap, play } = setup();
  play([0x90, 60, 100]);
  tap.setOutput("bang");
  tap.setOutput(MPE);
  assert.equal(tap.activeNotes(), 1);
  tap.setOutput(5); // anything else is MPE
  assert.equal(tap.activeNotes(), 1);
});

test("no stuck notes in Semitone output after fast playing in Gravity", () => {
  const { tap, play } = semitoneDevice();
  tap.setMode(GRAVITY);
  tap.setMod(6);
  let seed = 7;
  const random = () => (seed = (seed * 16807) % 2147483647) / 2147483647;
  const held = new Set();
  const sounding = new Map(); // note -> count of note ons minus note offs
  const track = (out) => {
    for (const [status, note] of messages(out)) {
      assert.equal(status & 0x0f, 0, "everything goes out on channel 1");
      if ((status & 0xf0) === 0x90) {
        assert.ok(!sounding.get(note), `second voice on ${note}`);
        sounding.set(note, 1);
      }
      if ((status & 0xf0) === 0x80) {
        assert.equal(sounding.get(note), 1, `note off for ${note} that is not sounding`);
        sounding.set(note, 0);
      }
    }
  };
  for (let i = 0; i < 2000; i++) {
    const note = 48 + Math.floor(random() * 12);
    if (held.has(note) && random() < 0.6) {
      held.delete(note);
      track(play([0x80, note, 0]));
    } else {
      held.add(note);
      track(play([0x90, note, 100]));
    }
    if (i % 97 === 0) track(play([0xb0, 64, random() < 0.5 ? 127 : 0]));
  }
  for (const note of held) track(play([0x80, note, 0]));
  assert.equal(tap.activeNotes(), 0);
  for (const [note, count] of sounding) assert.equal(count, 0, `${note} is stuck`);
});

// ---- Max glue: tap.js loaded as Max loads it, with a fake patcher ----

function loadInMax(panel) {
  const out = [];
  const drift = []; // each table sent to the drift view
  const max = {
    outlet: (index, value) => (index === 0 ? out.push(value) : drift.push(value)),
    patcher: {
      getnamed: (name) => (name in panel ? { getvalueof: () => panel[name] } : null),
    },
  };
  vm.runInNewContext(fs.readFileSync(path.join(__dirname, "../tap.js"), "utf8"), max);
  const play = (bytes) => {
    out.length = 0;
    bytes.forEach((byte) => max.msg_int(byte));
    return messages(out).map((m) => m.slice(1));
  };
  return { max, play, drift };
}

test("on load the device reads its settings from the panel", () => {
  const { play } = loadInMax({ mode: MOVE, mod: -2 });
  play([0x90, 60, 100, 0x80, 60, 0]);
  assert.deepEqual(play([0x90, 60, 100]).at(-1), [58, 100]);
});

test("on load the device reads Mod and the bounds from the panel", () => {
  const { play } = loadInMax({ mode: MOVE, mod: 12, lowest: 40, highest: 66 });
  play([0x90, 60, 100, 0x80, 60, 0]); // 72, held at 66
  assert.deepEqual(play([0x90, 60, 100]).at(-1), [66, 100]);
});

test("panel messages change Mod and the bounds", () => {
  const { max, play } = loadInMax({ mode: MOVE });
  max.mod(-12);
  max.lowest(55);
  max.highest(70);
  play([0x90, 60, 100, 0x80, 60, 0]); // 48, held at 55
  assert.deepEqual(play([0x90, 60, 100]).at(-1), [55, 100]);
});

test("a missing control leaves its setting at the default", () => {
  const { play } = loadInMax({ mode: MOVE });
  play([0x90, 60, 100, 0x80, 60, 0]);
  assert.deepEqual(play([0x90, 60, 100]).at(-1), [61, 100]);
});

test("panel messages change the mode and settings, and only a press resets", () => {
  const { max, play } = loadInMax({});
  max.mode(MOVE);
  max.mod(2);
  play([0x90, 60, 100, 0x80, 60, 0]);
  max.reset(0); // button let go
  assert.deepEqual(play([0x90, 60, 100, 0x80, 60, 0]).at(1), [62, 100]); // reset ignored
  max.reset(1);
  assert.deepEqual(play([0x90, 60, 100]).at(-1), [60, 100]);
  max.reset("bang");
  max.mode(SWAP);
  max.mod(1);
  play([0x80, 60, 0, 0x90, 67, 100, 0x80, 67, 0]);
  assert.deepEqual(play([0x90, 60, 100]).at(-1), [67, 100]);
});

test("on load the device reads Effect Range from the panel, and panel messages change it", () => {
  // Real randomness here, so only the spread is checked: Mod 0, Effect Range 2 stays within a semitone.
  const { max, play, drift } = loadInMax({ mode: MOVE, mod: 0, range: 2 });
  play([0x90, 60, 100, 0x80, 60, 0]);
  assert.ok(Math.abs(drift.at(-1)[60]) <= 100);
  assert.notEqual(drift.at(-1)[60], 0);
  max.range(0);
  play([0x90, 62, 100, 0x80, 62, 0]);
  assert.equal(drift.at(-1)[62], 0);
});

test("on load the device reads Output from the panel, and panel messages change it", () => {
  const { max, play } = loadInMax({ output: SEMITONE });
  assert.deepEqual(play([0x90, 60, 100]), [[60, 100]]); // plain note on, no bend
  max.output(MPE);
  play([0x80, 60, 0]);
  assert.equal(play([0x90, 62, 100]).length, 2); // bend then note
});

test("on load the device sends the drift view its table, and again after each release", () => {
  const { play, drift } = loadInMax({ mode: MOVE });
  const sent = drift.length;
  assert.ok(sent > 0);
  assert.equal(drift.at(-1).length, 128);
  assert.ok(drift.at(-1).every((cents) => cents === 0));
  play([0x90, 60, 100]);
  assert.equal(drift.length, sent); // nothing on a note on
  play([0x80, 60, 0]);
  assert.equal(drift.length, sent + 1);
  assert.equal(drift.at(-1)[60], 100);
});

test("a panel reset clears the drift view", () => {
  const { max, play, drift } = loadInMax({ mode: MOVE });
  play([0x90, 60, 100, 0x80, 60, 0]);
  max.reset(1);
  assert.equal(drift.at(-1)[60], 0);
});

// ---- The patch: every control reaches the js ----

test("each panel control is wired through its prepend into the js", () => {
  const patcher = JSON.parse(fs.readFileSync(path.join(__dirname, "../TAP.maxpat"), "utf8")).patcher;
  const boxes = patcher.boxes.map((b) => b.box);
  const byId = Object.fromEntries(boxes.map((box) => [box.id, box]));
  const into = (id) => patcher.lines.filter((l) => l.patchline.source[0] === id).map((l) => byId[l.patchline.destination[0]]);
  for (const name of ["mode", "mod", "range", "lowest", "highest", "reset", "output"]) {
    const control = boxes.find((box) => box.varname === name);
    assert.ok(control, `no control named ${name}`);
    const prepend = into(control.id).find((box) => box.text === `prepend ${name}`);
    assert.ok(prepend, `${name} is not wired to prepend ${name}`);
    assert.ok(into(prepend.id).some((box) => box.text === "js tap.js"), `prepend ${name} does not reach the js`);
  }
  assert.equal(boxes.find((box) => box.varname === "semitones"), undefined);
  assert.equal(boxes.find((box) => box.varname === "strength"), undefined);
});

test("the Mode slider has three steps, named in engine order", () => {
  const patcher = JSON.parse(fs.readFileSync(path.join(__dirname, "../TAP.maxpat"), "utf8")).patcher;
  const mode = patcher.boxes.map((b) => b.box).find((box) => box.varname === "mode");
  assert.equal(mode.maxclass, "live.slider");
  const valueof = mode.saved_attribute_attributes.valueof;
  assert.equal(valueof.parameter_longname, "Mode");
  assert.equal(valueof.parameter_type, 2); // an enum, so it shows the name, not a number
  assert.deepEqual(valueof.parameter_enum, ["Swap", "Gravity", "Move"]);
  assert.equal(valueof.parameter_mmax, 2);
});

// ---- Drift: what the view is sent ----

function driftSetup() {
  let changes = 0;
  const out = [];
  const tap = createTap((byte) => out.push(byte), () => changes++);
  const play = (bytes) => bytes.forEach(tap.input);
  return { tap, play, changes: () => changes };
}

test("drift is each key's offset from concert pitch in cents", () => {
  const { tap, play } = driftSetup();
  tap.setMode(MOVE);
  tap.setMod(-2);
  play([0x90, 64, 100, 0x80, 64, 0]);
  const drift = tap.drift();
  assert.equal(drift.length, 128);
  assert.equal(drift[64], -200);
  assert.equal(drift.filter((cents) => cents !== 0).length, 1);
});

test("drift is rounded to a tenth of a cent", () => {
  const { tap, play } = driftSetup();
  tap.setMode(GRAVITY);
  play([0x90, 57, 100, 0x80, 57, 0, 0x90, 69, 100, 0x80, 69, 0]);
  const cents = tap.drift()[57];
  assert.ok(cents > 0);
  assert.equal(Math.round(cents * 10), cents * 10);
});

test("the view is told once per release, and not on note on", () => {
  const { play, changes } = driftSetup();
  play([0x90, 60, 100]);
  assert.equal(changes(), 0);
  play([0x80, 60, 0, 0x90, 62, 100, 0x80, 62, 0]);
  assert.equal(changes(), 2);
});

test("with the pedal down the view is told once, when the pedal lifts", () => {
  const { play, changes } = driftSetup();
  play([0xb0, 64, 127]);
  play([0x90, 60, 100, 0x80, 60, 0, 0x90, 62, 100, 0x80, 62, 0, 0x90, 64, 100, 0x80, 64, 0]);
  assert.equal(changes(), 0);
  play([0xb0, 64, 0]);
  assert.equal(changes(), 1);
  play([0xb0, 64, 127, 0xb0, 64, 0]); // nothing waiting
  assert.equal(changes(), 1);
});

test("reset and a mode change tell the view; other settings do not", () => {
  const { tap, changes } = driftSetup();
  tap.reset();
  assert.equal(changes(), 1);
  tap.setMode(GRAVITY);
  assert.equal(changes(), 2);
  tap.setMode(GRAVITY); // same mode again
  tap.setMod(2);
  tap.setRange(4);
  tap.setLowest(30);
  tap.setHighest(90);
  tap.setOutput(SEMITONE);
  assert.equal(changes(), 2);
});
