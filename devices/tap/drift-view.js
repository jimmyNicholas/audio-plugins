// The visualiser for TAP: a square pitch map of all 128 keys. Runs in a jsui inside the
// panel window.
//
// Across is the key, note 0 at the left. Up is the pitch it plays, note 0 at the bottom, on
// a linear scale, so at concert pitch every key sits on the diagonal. Each key is one
// note-sized pixel on the diagonal.
//
// tap.js sends a list of 128 offsets in cents whenever the pitch table changes.
//
// Written in ES5 for Max 8's jsui. The tests load it in Node with a fake mgraphics.

autowatch = 1;
inlets = 1;
outlets = 0;

var NUM_NOTES = 128;
// Drift under this shows as concert pitch: the smallest step tap.js sends.
var MIN_CENTS = 0.1;
// The square's sizes, so each note is a whole number of pixels.
var SIZES = [128, 256, 384, 512];
// Room above the square for the summary line, when it is shown.
var HEADER_HEIGHT = 18;

var BACKGROUND = [0, 0, 0];
var CONCERT = [1, 1, 1];
var SHARP = [1, 0, 0];
var FLAT = [0, 0, 1];
var TEXT = [0.85, 0.85, 0.85];

// Where the square sits: pixels per note and its top left corner, under the header when
// it is shown.
function squareLayout(size, header) {
  return { perNote: size / NUM_NOTES, x: 0, y: header ? HEADER_HEIGHT : 0, size: size };
}

// The key's pixel on the diagonal: { x, y, width, height }.
function keyRect(note, cents, layout) {
  return {
    x: layout.x + note * layout.perNote,
    y: layout.y + (NUM_NOTES - 1 - note) * layout.perNote,
    width: layout.perNote,
    height: layout.perNote,
  };
}

// The stem from a key's pixel to the exact pitch it plays (note + cents / 100): up from the
// pixel's top when sharp, down from its bottom when flat, stopping at the square's edge.
// Null when shorter than a pixel.
function stemRect(note, cents, layout) {
  var pixel = keyRect(note, cents, layout);
  var length = (Math.abs(cents) / 100) * layout.perNote;
  if (!(length >= 1)) return null;
  var top = cents > 0 ? Math.max(layout.y, pixel.y - length) : pixel.y + pixel.height;
  var bottom = cents > 0 ? pixel.y : Math.min(layout.y + layout.size, top + length);
  if (bottom <= top) return null;
  return { x: pixel.x, y: top, width: pixel.width, height: bottom - top };
}

// White at concert pitch, else full red when sharp or full blue when flat.
function keyColour(cents) {
  if (!(Math.abs(cents) >= MIN_CENTS)) return CONCERT;
  return cents > 0 ? SHARP : FLAT;
}

// The header's summary line: how many keys have drifted.
function describe(drift) {
  var count = 0;
  for (var note = 0; note < drift.length; note++) {
    if (keyColour(drift[note]) !== CONCERT) count++;
  }
  if (count === 0) return "Every key at concert pitch";
  return count + (count === 1 ? " key" : " keys") + " drifted";
}

// ---- State and drawing ----

var drift = [];
for (var n = 0; n < NUM_NOTES; n++) drift.push(0);
var squareSize = 256;
var showSummary = false;

function boxHeight() {
  return squareSize + (showSummary ? HEADER_HEIGHT : 0);
}

// Grows or shrinks the jsui to fit the view, keeping its top left corner, in the window
// and in the patcher.
function fitBox() {
  var height = boxHeight();
  var shown = box.getattr("presentation_rect");
  box.setattr("presentation_rect", [shown[0], shown[1], squareSize, height]);
  box.rect = [box.rect[0], box.rect[1], box.rect[0] + squareSize, box.rect[1] + height];
}

function fillRect(rect, colour) {
  mgraphics.set_source_rgba(colour[0], colour[1], colour[2], 1);
  mgraphics.rectangle(rect.x, rect.y, rect.width, rect.height);
  mgraphics.fill();
}

function paint() {
  var layout = squareLayout(squareSize, showSummary);
  fillRect({ x: 0, y: 0, width: squareSize, height: boxHeight() }, BACKGROUND);
  if (showSummary) {
    mgraphics.set_source_rgba(TEXT[0], TEXT[1], TEXT[2], 1);
    mgraphics.select_font_face("Arial");
    mgraphics.set_font_size(11);
    mgraphics.move_to(4, 13);
    mgraphics.show_text(describe(drift));
  }
  for (var note = 0; note < NUM_NOTES; note++) {
    var colour = keyColour(drift[note]);
    fillRect(keyRect(note, drift[note], layout), colour);
    var stem = stemRect(note, drift[note], layout);
    if (stem) fillRect(stem, colour);
  }
}

// ---- Messages ----

// The drift table from tap.js: 128 offsets in cents.
function list() {
  var values = Array.prototype.slice.call(arguments);
  if (values.length !== NUM_NOTES) return;
  drift = values;
  mgraphics.redraw();
}

// From Debug: 128, 256, 384 or 512, snapped to the nearest.
function size(value) {
  var asked = parseFloat(value);
  if (isNaN(asked)) return;
  var next = SIZES[0];
  for (var i = 1; i < SIZES.length; i++) {
    if (Math.abs(SIZES[i] - asked) < Math.abs(next - asked)) next = SIZES[i];
  }
  if (next === squareSize) return;
  squareSize = next;
  fitBox();
  mgraphics.redraw();
}

// From Debug: 1 shows the header's summary line, 0 hides it.
function summary(value) {
  var next = Number(value) === 1;
  if (next === showSummary) return;
  showSummary = next;
  fitBox();
  mgraphics.redraw();
}

mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;
