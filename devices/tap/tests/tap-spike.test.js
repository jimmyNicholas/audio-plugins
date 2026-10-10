const test = require("node:test");
const assert = require("node:assert/strict");
const { createSpike, bendForCents, isBlackKey } = require("../spike/tap-spike.js");

function setup() {
  const out = [];
  const spike = createSpike((byte) => out.push(byte));
  const play = (bytes) => {
    out.length = 0;
    bytes.forEach(spike.input);
    return out.slice();
  };
  return { spike, play };
}

test("bend is centred for no detune and scaled to a 48 semitone range", () => {
  assert.equal(bendForCents(0), 8192);
  assert.equal(bendForCents(100), 8192 + 171); // 8192 / 48 = 170.67
  assert.equal(bendForCents(-100), 8192 - 171);
  assert.equal(bendForCents(4800), 16383);
  assert.equal(bendForCents(-4800), 0);
});

test("black keys", () => {
  assert.deepEqual(
    [60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71].filter(isBlackKey),
    [61, 63, 66, 68, 70],
  );
});

test("note on gets channel 2 with a bend sent first", () => {
  const { spike, play } = setup();
  spike.setDetune(100);
  const bend = bendForCents(100);
  assert.deepEqual(play([0x90, 61, 100]), [0xe1, bend & 0x7f, bend >> 7, 0x91, 61, 100]);
});

test("only black keys are detuned unless keys is set to all", () => {
  const { spike, play } = setup();
  spike.setDetune(50);
  assert.deepEqual(play([0x90, 60, 100]).slice(0, 3), [0xe1, 0, 64]);
  spike.setKeys(1);
  const bend = bendForCents(50);
  assert.deepEqual(play([0x90, 62, 100]).slice(0, 3), [0xe2, bend & 0x7f, bend >> 7]);
});

test("each held note gets its own channel and note off uses it", () => {
  const { play } = setup();
  assert.equal(play([0x90, 60, 100])[3], 0x91);
  assert.equal(play([0x90, 64, 100])[3], 0x92);
  assert.deepEqual(play([0x80, 60, 40]), [0x81, 60, 40]);
  assert.deepEqual(play([0x90, 64, 0]), [0x82, 64, 0]);
});

test("input channel is ignored", () => {
  const { play } = setup();
  assert.equal(play([0x95, 60, 100])[3], 0x91);
  assert.deepEqual(play([0x85, 60, 0]), [0x81, 60, 0]);
});

test("a new note takes the channel that has been free longest", () => {
  const { play } = setup();
  play([0x90, 60, 100]); // ch 2
  play([0x90, 62, 100]); // ch 3
  play([0x80, 62, 0]); // ch 3 free first
  play([0x80, 60, 0]); // then ch 2
  // Channels 4 to 16 have never been used, so they have been free longest.
  assert.equal(play([0x90, 64, 100])[3], 0x93);
});

test("with all 15 channels busy the oldest note is stopped", () => {
  const { play } = setup();
  for (let i = 0; i < 15; i++) play([0x90, 40 + i, 100]);
  assert.deepEqual(play([0x90, 80, 100]), [0x81, 40, 0, 0xe1, 0, 64, 0x91, 80, 100]);
  assert.deepEqual(play([0x80, 40, 0]), []); // already stopped
});

test("the same note played again is stopped first", () => {
  const { play } = setup();
  play([0x90, 60, 100]);
  assert.deepEqual(play([0x90, 60, 90]), [0x81, 60, 0, 0xe2, 0, 64, 0x92, 60, 90]);
});

test("shared messages go to channel 1", () => {
  const { play } = setup();
  assert.deepEqual(play([0xb3, 64, 127]), [0xb0, 64, 127]); // sustain pedal
  assert.deepEqual(play([0xe3, 0, 80]), [0xe0, 0, 80]); // pitch wheel
  assert.deepEqual(play([0xc3, 5]), [0xc0, 5]); // program change
});

test("running status is understood", () => {
  const { play } = setup();
  assert.deepEqual(play([0x90, 60, 100, 62, 100]).filter((b) => b >= 0x90 && b < 0xa0), [0x91, 0x92]);
});

test("poly aftertouch follows its note, real time and sysex pass through", () => {
  const { play } = setup();
  play([0x90, 60, 100]);
  assert.deepEqual(play([0xa0, 60, 30]), [0xa1, 60, 30]);
  assert.deepEqual(play([0xa0, 61, 30]), []);
  assert.deepEqual(play([0xf8]), [0xf8]);
  assert.deepEqual(play([0xf0, 1, 2, 0xf7]), [0xf0, 1, 2, 0xf7]);
});
