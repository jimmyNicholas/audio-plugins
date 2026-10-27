const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

const source = fs.readFileSync(path.join(__dirname, "../drift-view.js"), "utf8");

// Loads drift-view.js as a jsui would, with a fake mgraphics that records what is drawn.
function loadView() {
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
  const box = {
    rect: [640, 64, 896, 320], // left, top, right, bottom in the patcher
    attributes: { presentation_rect: [648, 12, 256, 256] }, // left, top, width, height in the window
    getattr(name) {
      return this.attributes[name];
    },
    setattr(name, value) {
      this.attributes[name] = value;
    },
  };
  const view = { mgraphics, box };
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

const WHITE = [1, 1, 1];
const RED = [1, 0, 0];
const BLUE = [0, 0, 1];

// The fills in a colour, as [x, y, width, height].
function filled(drawn, colour) {
  return drawn.fills.filter((f) => f.colour.join() === colour.join()).map((f) => f.rect);
}

test("concert pitch draws 128 white pixels on the diagonal and no stems", () => {
  const { view, drawn, paint } = loadView();
  view.list(...table({}));
  paint();
  const white = filled(drawn, WHITE);
  assert.equal(white.length, 128);
  // At 256 each note is 2 pixels: note 0 at the bottom left, note 127 at the top right.
  assert.deepEqual(white[0], [0, 254, 2, 2]);
  assert.deepEqual(white[60], [120, 134, 2, 2]);
  assert.deepEqual(white[127], [254, 0, 2, 2]);
  assert.equal(filled(drawn, RED).length + filled(drawn, BLUE).length, 0);
});

test("Move +3 on note 60 at size 256 draws a red pixel and a red stem 6 pixels up", () => {
  const { view, drawn, paint } = loadView();
  view.list(...table({ 60: 300 }));
  paint();
  assert.equal(filled(drawn, WHITE).length, 127);
  assert.deepEqual(filled(drawn, RED), [
    [120, 134, 2, 2], // the pixel, where note 60 sits on the diagonal
    [120, 128, 2, 6], // the stem, up to the pitch it plays: note 63's row
  ]);
});

test("a Swap pair draws one red stem up and one blue stem down, of equal length", () => {
  const { view, drawn, paint } = loadView();
  view.list(...table({ 69: 200, 71: -200 })); // A and B swapped
  paint();
  const [, redStem] = filled(drawn, RED);
  const [, blueStem] = filled(drawn, BLUE);
  assert.deepEqual(redStem, [138, 112, 2, 4]); // up from A's pixel to B's row
  assert.deepEqual(blueStem, [142, 114, 2, 4]); // down from B's pixel to A's row
});

test("0.1 cent sharp turns the pixel red with no stem; 0.05 cents stays white", () => {
  const { view, drawn, paint } = loadView();
  view.list(...table({ 60: 0.1, 62: 0.05, 64: -0.05 }));
  paint();
  assert.deepEqual(filled(drawn, RED), [[120, 134, 2, 2]]);
  assert.equal(filled(drawn, BLUE).length, 0);
  assert.equal(filled(drawn, WHITE).length, 127);
});

test("each size gives 1 to 4 pixels per note, and other sizes snap to the nearest", () => {
  const { view, drawn, paint } = loadView();
  for (const [sent, perNote] of [
    [128, 1],
    [256, 2],
    ["384", 3], // the Debug tab sends its text
    [512, 4],
    [300, 2],
    [330, 3],
    [10, 1],
    [2000, 4],
  ]) {
    view.size(sent);
    paint();
    assert.deepEqual(filled(drawn, WHITE)[127], [127 * perNote, 0, perNote, perNote], `size ${sent}`);
    assert.deepEqual(drawn.fills[0].rect, [0, 0, 128 * perNote, 128 * perNote], `size ${sent}: background`);
  }
});

test("a new size redraws the view and resizes its box from the same corner", () => {
  const { view, drawn } = loadView();
  view.size(384);
  assert.equal(drawn.redraws, 1);
  assert.deepEqual(Array.from(view.box.attributes.presentation_rect), [648, 12, 384, 384]);
  assert.deepEqual(Array.from(view.box.rect), [640, 64, 1024, 448]);
  view.size(400); // snaps to 384 again: nothing to do
  assert.equal(drawn.redraws, 1);
  view.size("big");
  assert.equal(drawn.redraws, 1);
});

test("the header is hidden by default and shows the summary when turned on", () => {
  const { view, drawn, paint } = loadView();
  paint();
  assert.deepEqual(drawn.text, []);
  assert.deepEqual(filled(drawn, WHITE)[127], [254, 0, 2, 2]);

  view.summary(1);
  assert.equal(drawn.redraws, 1);
  assert.deepEqual(Array.from(view.box.attributes.presentation_rect), [648, 12, 256, 274]); // room for the header
  paint();
  assert.deepEqual(drawn.text, ["Every key at concert pitch"]);
  assert.deepEqual(filled(drawn, WHITE)[127], [254, 18, 2, 2]); // the square sits under it

  view.list(...table({ 60: 300 }));
  paint();
  assert.deepEqual(drawn.text, ["1 key drifted"]);
  view.list(...table({ 60: 300, 62: -0.1, 64: 0.05 }));
  paint();
  assert.deepEqual(drawn.text, ["2 keys drifted"]);

  view.summary(0);
  paint();
  assert.deepEqual(drawn.text, []);
  assert.deepEqual(Array.from(view.box.attributes.presentation_rect), [648, 12, 256, 256]);
});

test("a stem stops at the edge of the square, clear of the header", () => {
  const { view, drawn, paint } = loadView();
  view.summary(1);
  view.list(...table({ 125: 1200, 2: -1200 }));
  paint();
  assert.deepEqual(filled(drawn, RED)[1], [250, 18, 2, 4]); // up to the top, not 12 notes
  assert.deepEqual(filled(drawn, BLUE)[1], [4, 270, 2, 4]); // down to the bottom
});

test("a list that is not 128 values is ignored", () => {
  const { view, drawn, paint } = loadView();
  view.list(1, 2, 3);
  assert.equal(drawn.redraws, 0);
  paint();
  assert.equal(filled(drawn, WHITE).length, 128);
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
