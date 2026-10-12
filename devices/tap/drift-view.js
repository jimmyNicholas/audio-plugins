// The drift view for TAP: a keyboard of all 128 keys showing how far each has drifted
// from concert pitch. Runs in a jsui inside the pop-out window.
//
// tap.js sends a list of 128 offsets in cents whenever the pitch table changes. Keys turn
// red when sharp and blue when flat, deeper the further they have gone, and a bar above
// each key shows the same. Hover over a key to read its offset.
//
// Written in ES5 for Max 8's jsui. The tests load it in Node with a fake mgraphics.

autowatch = 1;
inlets = 1;
outlets = 0;

var NUM_NOTES = 128;
var NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
var IS_BLACK = [false, true, false, true, false, false, true, false, true, false, true, false];

// Offsets this big or bigger get the deepest colour and the tallest bar.
var FULL_SCALE_CENTS = 1200;
// Offsets smaller than this are shown as no drift.
var MIN_CENTS = 0.5;

var HEADER_HEIGHT = 22;
var LANE_HEIGHT = 72;
var LABEL_HEIGHT = 14;

var BACKGROUND = [0.15, 0.15, 0.15];
var TEXT = [0.85, 0.85, 0.85];
var GRID = [0.32, 0.32, 0.32];
var WHITE_KEY = [0.92, 0.92, 0.9];
var BLACK_KEY = [0.12, 0.12, 0.12];
var SHARP = [0.9, 0.25, 0.2];
var FLAT = [0.2, 0.45, 0.95];
var HOVER = [1, 0.8, 0.2];

// As Live names them: note 60, middle C, is C3.
function noteName(note) {
  return NAMES[note % 12] + (Math.floor(note / 12) - 2);
}

function isBlack(note) {
  return IS_BLACK[note % 12];
}

// 0 for no drift up to 1 at FULL_SCALE_CENTS, on a log scale so a few cents still show.
function amount(cents) {
  var size = Math.abs(cents);
  if (!(size >= MIN_CENTS)) return 0;
  return Math.min(1, Math.log(1 + size / 10) / Math.log(1 + FULL_SCALE_CENTS / 10));
}

function mix(from, to, t) {
  return [from[0] + (to[0] - from[0]) * t, from[1] + (to[1] - from[1]) * t, from[2] + (to[2] - from[2]) * t];
}

// The key's colour: its natural colour mixed toward red if sharp or blue if flat.
function keyColour(note, cents) {
  var base = isBlack(note) ? BLACK_KEY : WHITE_KEY;
  var t = amount(cents);
  if (t === 0) return base;
  return mix(base, cents > 0 ? SHARP : FLAT, 0.25 + 0.75 * t);
}

// Where each key sits across the given width: { x, width, black } for notes 0 to 127.
function keyLayout(width) {
  var whites = 0;
  var note;
  for (note = 0; note < NUM_NOTES; note++) {
    if (!isBlack(note)) whites++;
  }
  var whiteWidth = width / whites;
  var blackWidth = whiteWidth * 0.6;
  var keys = [];
  var white = 0;
  for (note = 0; note < NUM_NOTES; note++) {
    if (isBlack(note)) {
      keys.push({ x: white * whiteWidth - blackWidth / 2, width: blackWidth, black: true });
    } else {
      keys.push({ x: white * whiteWidth, width: whiteWidth, black: false });
      white++;
    }
  }
  return keys;
}

// The key under a point on the keyboard. Black keys sit on top, so they are checked first.
// blackHeight is how far down the black keys reach from the top of the keyboard.
function keyAt(keys, x, yInKeyboard, blackHeight) {
  var note;
  if (yInKeyboard < blackHeight) {
    for (note = 0; note < keys.length; note++) {
      if (keys[note].black && x >= keys[note].x && x < keys[note].x + keys[note].width) return note;
    }
  }
  for (note = 0; note < keys.length; note++) {
    if (!keys[note].black && x >= keys[note].x && x < keys[note].x + keys[note].width) return note;
  }
  return -1;
}

// The key whose bar is closest to x, for hovering over the bars.
function nearestKey(keys, x) {
  var best = -1;
  var bestDistance = Infinity;
  for (var note = 0; note < keys.length; note++) {
    var distance = Math.abs(keys[note].x + keys[note].width / 2 - x);
    if (distance < bestDistance) {
      best = note;
      bestDistance = distance;
    }
  }
  return best;
}

function formatCents(cents) {
  return (cents > 0 ? "+" : "") + cents.toFixed(1) + " cents";
}

// The line at the top: the hovered key, or how many keys have drifted.
function describe(drift, hovered) {
  if (hovered >= 0) {
    var cents = drift[hovered];
    var text = noteName(hovered) + "   " + formatCents(cents);
    var nearest = Math.round(hovered + cents / 100);
    if (nearest !== hovered) text += "   (plays near " + (nearest >= 0 ? noteName(nearest) : "below C-2") + ")";
    return text;
  }
  var count = 0;
  for (var note = 0; note < drift.length; note++) {
    if (amount(drift[note]) > 0) count++;
  }
  if (count === 0) return "Every key at concert pitch";
  return count + (count === 1 ? " key has" : " keys have") + " drifted. Hover over a key to see by how much.";
}

// ---- State and drawing ----

var drift = [];
for (var n = 0; n < NUM_NOTES; n++) drift.push(0);
var hovered = -1;

function size() {
  return [box.rect[2] - box.rect[0], box.rect[3] - box.rect[1]];
}

function keyboardTop() {
  return HEADER_HEIGHT + LANE_HEIGHT;
}

function keyboardHeight() {
  return size()[1] - keyboardTop() - LABEL_HEIGHT;
}

function setColour(colour) {
  mgraphics.set_source_rgba(colour[0], colour[1], colour[2], 1);
}

function fillRect(x, y, width, height, colour) {
  setColour(colour);
  mgraphics.rectangle(x, y, width, height);
  mgraphics.fill();
}

function outlineRect(x, y, width, height, colour) {
  setColour(colour);
  mgraphics.set_line_width(2);
  mgraphics.rectangle(x + 1, y + 1, width - 2, height - 2);
  mgraphics.stroke();
}

function drawText(text, x, y, fontSize) {
  mgraphics.select_font_face("Arial");
  mgraphics.set_font_size(fontSize);
  mgraphics.move_to(x, y);
  mgraphics.show_text(text);
}

// Bars above the keys: up for sharp, down for flat, from a centre line.
function drawBars(keys, width) {
  var middle = HEADER_HEIGHT + LANE_HEIGHT / 2;
  var reach = LANE_HEIGHT / 2 - 3;
  fillRect(0, middle, width, 1, GRID);
  for (var note = 0; note < NUM_NOTES; note++) {
    var t = amount(drift[note]);
    if (t === 0) continue;
    var barWidth = Math.max(2, keys[note].width * 0.6);
    var x = keys[note].x + (keys[note].width - barWidth) / 2;
    var height = Math.max(1, t * reach);
    fillRect(x, drift[note] > 0 ? middle - height : middle + 1, barWidth, height, drift[note] > 0 ? SHARP : FLAT);
  }
}

function drawKeyboard(keys) {
  var top = keyboardTop();
  var height = keyboardHeight();
  var blackHeight = height * 0.62;
  var note;
  for (note = 0; note < NUM_NOTES; note++) {
    if (keys[note].black) continue;
    fillRect(keys[note].x, top, keys[note].width, height, keyColour(note, drift[note]));
    fillRect(keys[note].x, top, 1, height, GRID);
    if (note % 12 === 0) {
      setColour(TEXT);
      drawText(noteName(note), keys[note].x + 1, top + height + 11, 9);
    }
  }
  for (note = 0; note < NUM_NOTES; note++) {
    if (!keys[note].black) continue;
    fillRect(keys[note].x, top, keys[note].width, blackHeight, keyColour(note, drift[note]));
  }
  if (hovered >= 0) {
    var key = keys[hovered];
    outlineRect(key.x, top, key.width, key.black ? blackHeight : height, HOVER);
  }
}

function paint() {
  var width = size()[0];
  var keys = keyLayout(width);
  fillRect(0, 0, width, size()[1], BACKGROUND);
  setColour(TEXT);
  drawText(describe(drift, hovered), 6, 15, 12);
  drawBars(keys, width);
  drawKeyboard(keys);
}

// ---- Messages ----

// The drift table from tap.js: 128 offsets in cents.
function list() {
  var values = Array.prototype.slice.call(arguments);
  if (values.length !== NUM_NOTES) return;
  drift = values;
  mgraphics.redraw();
}

function hover(x, y) {
  var keys = keyLayout(size()[0]);
  var next = -1;
  if (y >= keyboardTop()) {
    next = keyAt(keys, x, y - keyboardTop(), keyboardHeight() * 0.62);
  } else if (y >= HEADER_HEIGHT) {
    next = nearestKey(keys, x);
  }
  if (next === hovered) return;
  hovered = next;
  mgraphics.redraw();
}

function onidle(x, y) {
  hover(x, y);
}

function onclick(x, y) {
  hover(x, y);
}

function onidleout() {
  if (hovered === -1) return;
  hovered = -1;
  mgraphics.redraw();
}

mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;
