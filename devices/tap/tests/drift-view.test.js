const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

const source = fs.readFileSync(path.join(__dirname, "../drift-view.js"), "utf8");

// Loads drift-view.js as a jsui would, with a fake mgraphics that records what is drawn.
function loadView(width = 960, height = 200) {
  const drawn = { text: [], fills: [], redraws: 0 };
  const mgraphics = {
    init() {},
    redraw() {
      drawn.redraws++;
    },
    set_source_rgba(r, g, b) {
      this.colour = [r, g, b];
    },
    rectangle(x, y, w, h) {
      this.rect = [x, y, w, h];
    },
    fill() {
      drawn.fills.push({ rect: this.rect, colour: this.colour });
    },
    stroke() {},
    set_line_width() {},
    select_font_face() {},
    set_font_size() {},
    move_to() {},
    show_text(text) {
      drawn.text.push(text);
    },
  };
  const view = { mgraphics, box: { rect: [0, 0, width, height] } };
  vm.runInNewContext(source, view);
  const paint = () => {
    drawn.text.length = 0;
    drawn.fills.length = 0;
    view.paint();
  };
  return { view, drawn, paint };
}

function table(changes) {
  const cents = new Array(128).fill(0);
  for (const note in changes) cents[note] = changes[note];
  return cents;
}

test("notes are named as Live names them, middle C is C3", () => {
  const { view } = loadView();
  assert.equal(view.noteName(60), "C3");
  assert.equal(view.noteName(0), "C-2");
  assert.equal(view.noteName(61), "C#3");
  assert.equal(view.noteName(127), "G8");
});

test("the keyboard spans the width with 75 white keys and black keys between them", () => {
  const { view } = loadView();
  const keys = view.keyLayout(750);
  assert.equal(keys.length, 128);
  assert.equal(keys.filter((key) => !key.black).length, 75);
  assert.equal(keys[0].x, 0);
  assert.equal(keys[127].x + keys[127].width, 750);
  assert.ok(keys[61].black);
  assert.equal(keys[61].x + keys[61].width / 2, keys[62].x); // C# sits on the C to D line
});

test("hovering finds black keys over white ones, and white keys below the black ones", () => {
  const { view } = loadView();
  const keys = view.keyLayout(750);
  const line = keys[62].x; // between C3 and D3, under C#3
  assert.equal(view.keyAt(keys, line, 5, 50), 61);
  assert.equal(view.keyAt(keys, line + 1, 70, 50), 62);
  assert.equal(view.keyAt(keys, -5, 70, 50), -1);
  assert.equal(view.nearestKey(keys, line), 61);
});

test("colour deepens with drift, red for sharp and blue for flat", () => {
  const { view } = loadView();
  assert.equal(view.amount(0), 0);
  assert.equal(view.amount(0.2), 0);
  assert.ok(view.amount(5) > 0);
  assert.ok(view.amount(50) < view.amount(100));
  assert.equal(view.amount(1200), 1);
  assert.equal(view.amount(-5000), 1);
  const sharp = view.keyColour(60, 100);
  const flat = view.keyColour(60, -100);
  assert.ok(sharp[0] > sharp[2]);
  assert.ok(flat[2] > flat[0]);
  assert.deepEqual(Array.from(view.keyColour(60, 0)), [0.92, 0.92, 0.9]);
});

test("the top line counts drifted keys, or describes the hovered one", () => {
  const { view } = loadView();
  assert.equal(view.describe(table({}), -1), "Every key at concert pitch");
  assert.match(view.describe(table({ 60: 12 }), -1), /^1 key has drifted/);
  assert.match(view.describe(table({ 60: 12, 64: -3 }), -1), /^2 keys have drifted/);
  assert.equal(view.describe(table({ 60: 12.5 }), 60), "C3   +12.5 cents");
  assert.equal(view.describe(table({ 60: -200 }), 60), "C3   -200.0 cents   (plays near A#2)");
});

test("a new table redraws the view and paints each drifted key", () => {
  const { view, drawn, paint } = loadView();
  view.list(...table({ 60: 100, 64: -50 }));
  assert.equal(drawn.redraws, 1);
  paint();
  assert.match(drawn.text[0], /^2 keys have drifted/);
  assert.ok(drawn.text.includes("C3")); // octave labels
  const red = drawn.fills.filter((f) => f.colour[0] === 0.9 && f.colour[1] === 0.25);
  const blue = drawn.fills.filter((f) => f.colour[2] === 0.95 && f.colour[1] === 0.45);
  assert.equal(red.length, 1); // C3's bar
  assert.equal(blue.length, 1); // E3's bar
});

test("a list that is not 128 values is ignored", () => {
  const { view, drawn, paint } = loadView();
  view.list(1, 2, 3);
  assert.equal(drawn.redraws, 0);
  paint();
  assert.equal(drawn.text[0], "Every key at concert pitch");
});

test("hovering a key shows its drift until the mouse leaves", () => {
  const { view, drawn, paint } = loadView(750, 200);
  view.list(...table({ 60: 100 }));
  const keys = view.keyLayout(750);
  view.onidle(keys[60].x + 2, 190);
  paint();
  assert.equal(drawn.text[0], "C3   +100.0 cents   (plays near C#3)");
  view.onidle(keys[60].x + 2, 190); // same key, no extra redraw
  assert.equal(drawn.redraws, 2);
  view.onidleout();
  paint();
  assert.match(drawn.text[0], /^1 key has drifted/);
});

// ---- Picture mode ----

const same = (a, b) => a.every((v, i) => Math.abs(v - b[i]) < 1e-9);

test("the landscape has a colour for every square", () => {
  const { view } = loadView();
  for (let square = 0; square < 128; square++) {
    const colour = view.squareColour(square);
    assert.ok(Array.isArray(colour) && colour.length === 3, `square ${square}`);
  }
  assert.deepEqual(Array.from(view.squareColour(0)), [0.24, 0.44, 0.78]); // sky, top left
  assert.deepEqual(Array.from(view.squareColour(127)), [0.42, 0.28, 0.15]); // tree trunk, bottom right
});

test("at concert pitch every slot shows its own square", () => {
  const { view } = loadView();
  for (let slot = 0; slot < 128; slot++) {
    assert.deepEqual({ ...view.slotSquares(slot, 0) }, { from: slot, to: slot, t: 0 });
  }
  assert.deepEqual({ ...view.slotSquares(40, 0.3) }, { from: 40, to: 40, t: 0 }); // too small to show
});

test("whole note drift shows another square: Swap trades, Move shifts", () => {
  const { view } = loadView();
  // Swap 69 and 71: each slot shows the other's square.
  assert.deepEqual({ ...view.slotSquares(69, 200) }, { from: 71, to: 71, t: 0 });
  assert.deepEqual({ ...view.slotSquares(71, -200) }, { from: 69, to: 69, t: 0 });
  // Move by one, and by one more after floating point drift.
  assert.deepEqual({ ...view.slotSquares(60, 100) }, { from: 61, to: 61, t: 0 });
  assert.deepEqual({ ...view.slotSquares(60, 199.99) }, { from: 62, to: 62, t: 0 });
});

test("drift between notes blends the two nearest squares", () => {
  const { view } = loadView();
  const squares = view.slotSquares(16, 25);
  assert.equal(squares.from, 16);
  assert.equal(squares.to, 17);
  assert.ok(Math.abs(squares.t - 0.25) < 1e-9);
  const flat = view.slotSquares(16, -30);
  assert.equal(flat.from, 15);
  assert.equal(flat.to, 16);
  assert.ok(Math.abs(flat.t - 0.7) < 1e-9);
  // Square 35 is low sky and 36 is cloud, so half a semitone sharp is halfway between.
  const half = view.slotColour(35, 50);
  assert.ok(same(half, view.mix(view.squareColour(35), view.squareColour(36), 0.5)));
  assert.ok(!same(half, view.squareColour(35)) && !same(half, view.squareColour(36)));
});

test("pitches past either end show the end square", () => {
  const { view } = loadView();
  assert.deepEqual({ ...view.slotSquares(2, -5000) }, { from: 0, to: 0, t: 0 });
  assert.deepEqual({ ...view.slotSquares(125, 1200) }, { from: 127, to: 127, t: 0 });
});

test("the picture is centred under the header and slots are found by position", () => {
  const { view } = loadView();
  const layout = view.pictureLayout(960, 200);
  assert.equal(layout.square, 21);
  assert.equal(layout.x, (960 - 21 * 16) / 2);
  assert.ok(layout.y >= 22 && layout.y + 21 * 8 <= 200);
  assert.equal(view.slotAt(layout, layout.x, layout.y), 0);
  assert.equal(view.slotAt(layout, layout.x + 21 * 15 + 1, layout.y + 21 * 7 + 1), 127);
  assert.equal(view.slotAt(layout, layout.x + 21 * 3 + 5, layout.y + 21 + 5), 19);
  assert.equal(view.slotAt(layout, layout.x - 1, layout.y), -1);
  assert.equal(view.slotAt(layout, layout.x, layout.y + 21 * 8), -1);
});

function clickButton(view, name) {
  const button = view.buttonLayout(960).find((b) => b.name === name);
  view.onclick(button.x + 5, 10);
}

test("the switch toggles between keys and picture", () => {
  const { view, drawn, paint } = loadView();
  paint();
  assert.ok(drawn.text.includes("Keys") && drawn.text.includes("Picture"));
  assert.ok(drawn.text.includes("C3")); // keyboard labels

  clickButton(view, "picture");
  assert.equal(drawn.redraws, 1);
  paint();
  assert.ok(!drawn.text.includes("C3"));
  const layout = view.pictureLayout(960, 200);
  const squares = drawn.fills.filter((f) => f.rect[2] === layout.square && f.rect[3] === layout.square);
  assert.equal(squares.length, 128);
  assert.deepEqual(squares[0].colour, Array.from(view.squareColour(0)));

  clickButton(view, "picture"); // already on, nothing to do
  assert.equal(drawn.redraws, 1);
  clickButton(view, "keys");
  paint();
  assert.ok(drawn.text.includes("C3"));
});

test("a swap trades two squares in the drawn picture", () => {
  const { view, drawn, paint } = loadView();
  clickButton(view, "picture");
  view.list(...table({ 29: 200, 31: -200 })); // a sun square and a sky square trade places
  paint();
  const layout = view.pictureLayout(960, 200);
  const squares = drawn.fills.filter((f) => f.rect[2] === layout.square && f.rect[3] === layout.square);
  assert.deepEqual(squares[29].colour, Array.from(view.squareColour(31)));
  assert.deepEqual(squares[31].colour, Array.from(view.squareColour(29)));
  assert.notDeepEqual(squares[29].colour, Array.from(view.squareColour(29)));
  assert.deepEqual(squares[30].colour, Array.from(view.squareColour(30)));
});

test("hovering a square in picture mode describes its key", () => {
  const { view, drawn, paint } = loadView();
  clickButton(view, "picture");
  view.list(...table({ 60: 100 }));
  const layout = view.pictureLayout(960, 200);
  view.onidle(layout.x + (60 % 16) * layout.square + 2, layout.y + Math.floor(60 / 16) * layout.square + 2);
  paint();
  assert.equal(drawn.text[0], "C3   +100.0 cents   (plays near C#3)");
  view.onidle(layout.x - 10, layout.y);
  paint();
  assert.match(drawn.text[0], /^1 key has drifted/);
});

// Max 8's js and jsui run ES5, so later syntax would only fail inside Live.
for (const file of ["tap.js", "drift-view.js"]) {
  test(`${file} uses only ES5 syntax`, () => {
    const code = fs
      .readFileSync(path.join(__dirname, "..", file), "utf8")
      .replace(/\/\/.*$/gm, "")
      .replace(/"(?:[^"\\]|\\.)*"/g, '""');
    for (const [pattern, name] of [
      [/\b(let|const|class)\s/, "let, const or class"],
      [/=>/, "arrow function"],
      [/`/, "template literal"],
      [/\.\.\./, "spread"],
      [/\b(Map|Set|Promise|Symbol)\b/, "ES6 built in"],
    ]) {
      assert.doesNotMatch(code, pattern, `${file} uses ${name}`);
    }
  });
}
