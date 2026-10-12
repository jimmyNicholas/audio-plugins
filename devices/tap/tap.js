// The Annoying Piano as a Max for Live MIDI effect.
//
// Every key you release changes the pitch of keys you play later. Reads raw MIDI bytes
// from [midiin] and writes them for [midiout] in one of two ways, chosen by Output.
// MPE, so each note can have its own pitch:
// - each note on gets its own channel, 2 to 16, with a pitch bend sent just before it
// - note off goes out on the note's channel
// - everything else goes out on channel 1, the MPE master channel
// Semitone, for instruments without MPE: plain notes on channel 1, each pitch rounded to
// the nearest semitone.
// The second outlet sends each key's drift from concert pitch, in cents, to the drift view
// whenever the pitch table changes.
//
// Written in ES5 for Max 8's js object. Also loads in Node for the tests.

autowatch = 1;
inlets = 1;
outlets = 2;

var NUM_NOTES = 128;
var BEND_RANGE_SEMITONES = 48; // MPE default; ticket 01 showed Live agrees
var FIRST_MEMBER_CHANNEL = 1; // zero-based, so MIDI channel 2
var LAST_MEMBER_CHANNEL = 15; // MIDI channel 16
var CC_SUSTAIN = 64;
var CC_ALL_SOUND_OFF = 120;
var CC_ALL_NOTES_OFF = 123;

// Modes, in the order of the panel's Mode tab.
var SWAP = 0;
var GRAVITY = 1;
var MOVE = 2;
var MAX_STRENGTH = 10;

// Outputs, in the order of the panel's Output tab.
var MPE = 0;
var SEMITONE = 1;

function hertzForPitch(pitch) {
  return 440 * Math.pow(2, (pitch - 69) / 12);
}

function pitchForHertz(hertz) {
  return 69 + (12 * Math.log(hertz / 440)) / Math.LN2;
}

function isNumber(value) {
  return typeof value === "number" && !isNaN(value);
}

function clamp(value, low, high) {
  return Math.max(low, Math.min(high, value));
}

// 14-bit pitch bend value (8192 is centre) for an offset in cents.
function bendForCents(cents) {
  var value = 8192 + Math.round((cents * 8192) / (BEND_RANGE_SEMITONES * 100));
  return Math.max(0, Math.min(16383, value));
}

// Number of data bytes that follow a status byte.
function dataLength(status) {
  var type = status & 0xf0;
  if (type === 0xc0 || type === 0xd0) return 1;
  if (type < 0xf0) return 2;
  if (status === 0xf1 || status === 0xf3) return 1;
  if (status === 0xf2) return 2;
  return 0;
}

// The pitch table and the modes. Each key's pitch is stored in semitones from MIDI
// note 0, so concert pitch is the note number itself. onChange, if given, is called once
// after each change to the table: a release, a pedal lift with releases waiting, a reset.
function createEngine(onChange) {
  var pitches = [];
  var lastReleased = -1;
  var pedalDown = false;
  var queue = [];
  var mode = SWAP;
  var strength = 2;
  var semitones = 1;

  function changed() {
    if (onChange) onChange();
  }

  function clear() {
    for (var note = 0; note < NUM_NOTES; note++) pitches[note] = note;
    lastReleased = -1;
    queue = [];
  }

  // Releases still waiting for the pedal are dropped too, so lifting it changes nothing.
  function reset() {
    clear();
    changed();
  }

  function swap(last, current) {
    var swapped = pitches[last];
    pitches[last] = pitches[current];
    pitches[current] = swapped;
  }

  // As in gravity.ts, in Hertz. Strength 10 divides by zero there, so here it pulls the
  // last key exactly onto the current key. Just below 10 the web formula overshoots a long
  // way, so the result is kept at or above note 0.
  function gravity(last, current) {
    if (strength >= MAX_STRENGTH) {
      pitches[last] = pitches[current];
      return;
    }
    var lastHertz = hertzForPitch(pitches[last]);
    var currentHertz = hertzForPitch(pitches[current]);
    var hertz = lastHertz - (lastHertz - currentHertz) / (MAX_STRENGTH - strength);
    pitches[last] = hertz > hertzForPitch(0) ? pitchForHertz(hertz) : 0;
  }

  function apply(current) {
    // As in useKeyboard.ts: on the very first release the last key is the current key.
    var last = lastReleased < 0 ? current : lastReleased;
    if (mode === MOVE) pitches[current] += semitones;
    else if (last !== current && mode === GRAVITY) gravity(last, current);
    else if (last !== current) swap(last, current);
    lastReleased = current;
  }

  // Changing mode resets every key, as in the web app. The same mode again does not.
  function setMode(value) {
    if (!isNumber(value)) return;
    var next = Math.round(value);
    if (next !== SWAP && next !== GRAVITY && next !== MOVE) next = SWAP;
    if (next === mode) return;
    mode = next;
    reset();
  }

  function noteReleased(note) {
    if (note < 0 || note >= NUM_NOTES) return;
    if (pedalDown) {
      queue.push(note);
      return;
    }
    apply(note);
    changed();
  }

  function setPedal(down) {
    pedalDown = down;
    if (down || queue.length === 0) return;
    for (var i = 0; i < queue.length; i++) apply(queue[i]);
    queue = [];
    changed();
  }

  clear();

  return {
    reset: reset,
    noteReleased: noteReleased,
    setPedal: setPedal,
    setMode: setMode,
    setStrength: function (value) {
      if (isNumber(value)) strength = clamp(value, 1, MAX_STRENGTH);
    },
    setSemitones: function (value) {
      if (isNumber(value)) semitones = clamp(Math.round(value), -2, 2);
    },
    mode: function () {
      return mode;
    },
    strength: function () {
      return strength;
    },
    semitones: function () {
      return semitones;
    },
    pitchForNote: function (note) {
      return pitches[note];
    },
    // Each key's offset from concert pitch in cents, to a tenth of a cent.
    drift: function () {
      var cents = [];
      for (var note = 0; note < NUM_NOTES; note++) {
        cents.push(Math.round((pitches[note] - note) * 1000) / 10);
      }
      return cents;
    },
  };
}

// Gives each note its own member channel. send(byte) is called for every output byte.
// Notes are looked up by key, the note that came in, since the note sent out can differ.
function createRouter(send) {
  // One voice per member channel. key is -1 when free. order is when the note started,
  // or when the channel was freed, so we can steal the oldest and reuse the longest free.
  var voices = [];
  var counter = 0;
  for (var ch = FIRST_MEMBER_CHANNEL; ch <= LAST_MEMBER_CHANNEL; ch++) {
    voices.push({ channel: ch, key: -1, note: 0, order: 0 });
  }

  function findVoice(key) {
    for (var i = 0; i < voices.length; i++) {
      if (voices[i].key === key) return voices[i];
    }
    return null;
  }

  function stop(voice, velocity) {
    send(0x80 | voice.channel);
    send(voice.note);
    send(velocity);
    voice.key = -1;
    voice.order = ++counter;
  }

  function chooseVoice() {
    var best = null;
    var i;
    for (i = 0; i < voices.length; i++) {
      if (voices[i].key === -1 && (best === null || voices[i].order < best.order)) best = voices[i];
    }
    if (best !== null) return best;
    for (i = 0; i < voices.length; i++) {
      if (best === null || voices[i].order < best.order) best = voices[i];
    }
    stop(best, 0);
    return best;
  }

  // Sends the nearest whole note, with a bend for the rest of the pitch.
  function noteOn(key, pitch, velocity) {
    var voice = chooseVoice();
    var note = Math.max(0, Math.min(NUM_NOTES - 1, Math.round(pitch)));
    var bend = bendForCents((pitch - note) * 100);
    send(0xe0 | voice.channel);
    send(bend & 0x7f);
    send(bend >> 7);
    send(0x90 | voice.channel);
    send(note);
    send(velocity);
    voice.key = key;
    voice.note = note;
    voice.order = ++counter;
  }

  // Returns false if the key has no note, for example because it was stolen.
  function noteOff(key, velocity) {
    var voice = findVoice(key);
    if (voice === null) return false;
    stop(voice, velocity);
    return true;
  }

  function aftertouch(key, pressure) {
    var voice = findVoice(key);
    if (voice === null) return;
    send(0xa0 | voice.channel);
    send(voice.note);
    send(pressure);
  }

  function allNotesOff() {
    for (var i = 0; i < voices.length; i++) {
      if (voices[i].key !== -1) stop(voices[i], 0);
    }
  }

  return {
    noteOn: noteOn,
    noteOff: noteOff,
    aftertouch: aftertouch,
    allNotesOff: allNotesOff,
    activeNotes: function () {
      var count = 0;
      for (var i = 0; i < voices.length; i++) {
        if (voices[i].key !== -1) count++;
      }
      return count;
    },
  };
}

// Plain MIDI on channel 1 for instruments without MPE. Each pitch is rounded to the nearest
// semitone, so two held keys can share a note. The second key restarts the note (a note
// off, then its note on) so the instrument never has two voices on one note, and the note
// off goes out only when the last of the keys is released. Same interface as createRouter.
function createSemitoneRouter(send) {
  var noteForKey = []; // -1 when the key has no note
  var keysOnNote = []; // how many keys hold each note
  for (var i = 0; i < NUM_NOTES; i++) {
    noteForKey[i] = -1;
    keysOnNote[i] = 0;
  }

  function noteOn(key, pitch, velocity) {
    var note = Math.max(0, Math.min(NUM_NOTES - 1, Math.round(pitch)));
    if (keysOnNote[note] > 0) {
      send(0x80);
      send(note);
      send(0);
    }
    send(0x90);
    send(note);
    send(velocity);
    noteForKey[key] = note;
    keysOnNote[note]++;
  }

  // Returns false if the key has no note.
  function noteOff(key, velocity) {
    var note = noteForKey[key];
    if (note === -1) return false;
    noteForKey[key] = -1;
    keysOnNote[note]--;
    if (keysOnNote[note] === 0) {
      send(0x80);
      send(note);
      send(velocity);
    }
    return true;
  }

  function aftertouch(key, pressure) {
    var note = noteForKey[key];
    if (note === -1) return;
    send(0xa0);
    send(note);
    send(pressure);
  }

  function allNotesOff() {
    for (var key = 0; key < NUM_NOTES; key++) {
      if (noteForKey[key] !== -1) noteOff(key, 0);
    }
  }

  return {
    noteOn: noteOn,
    noteOff: noteOff,
    aftertouch: aftertouch,
    allNotesOff: allNotesOff,
    // Keys holding a note, so two keys on one note count twice.
    activeNotes: function () {
      var count = 0;
      for (var key = 0; key < NUM_NOTES; key++) {
        if (noteForKey[key] !== -1) count++;
      }
      return count;
    },
  };
}

// The whole device: parses MIDI bytes, retunes notes and routes them. onDrift, if given,
// is called after each change to the pitch table; read the table with drift().
function createTap(send, onDrift) {
  var engine = createEngine(onDrift);
  var routers = [createRouter(send), createSemitoneRouter(send)];
  var output = MPE;
  var router = routers[output];

  // Keys that are down, so a stray note off (say the device loaded while a key was held)
  // does not count as a release.
  var keysDown = [];
  for (var key = 0; key < NUM_NOTES; key++) keysDown[key] = false;

  // Byte parser state.
  var status = 0;
  var data = [];
  var sysex = null;

  function sendAll(bytes) {
    for (var i = 0; i < bytes.length; i++) send(bytes[i]);
  }

  function noteOn(key, velocity) {
    // The same key again while held: stop the old note, but it is not a release.
    router.noteOff(key, 0);
    keysDown[key] = true;
    router.noteOn(key, engine.pitchForNote(key), velocity);
  }

  function noteOff(key, velocity) {
    if (!keysDown[key]) return;
    keysDown[key] = false;
    router.noteOff(key, velocity);
    engine.noteReleased(key);
  }

  function controller(number, value) {
    if (number === CC_SUSTAIN) {
      engine.setPedal(value >= 64);
    } else if (number === CC_ALL_NOTES_OFF || number === CC_ALL_SOUND_OFF) {
      router.allNotesOff();
      for (var key = 0; key < NUM_NOTES; key++) keysDown[key] = false;
    }
    sendAll([0xb0, number, value]);
  }

  // Held notes are stopped first so none get stuck. Their keys still count as released
  // when they come up, as with a stolen note.
  function setOutput(value) {
    if (!isNumber(value)) return;
    var next = Math.round(value) === SEMITONE ? SEMITONE : MPE;
    if (next === output) return;
    router.allNotesOff();
    output = next;
    router = routers[output];
  }

  function handleMessage(bytes) {
    var type = bytes[0] & 0xf0;
    if (type === 0x90 && bytes[2] > 0) {
      noteOn(bytes[1], bytes[2]);
    } else if (type === 0x80 || type === 0x90) {
      noteOff(bytes[1], type === 0x80 ? bytes[2] : 0);
    } else if (type === 0xa0) {
      router.aftertouch(bytes[1], bytes[2]);
    } else if (type === 0xb0) {
      controller(bytes[1], bytes[2]);
    } else if (type < 0xf0) {
      bytes[0] = type; // channel 1
      sendAll(bytes);
    } else {
      sendAll(bytes);
    }
  }

  function input(byte) {
    if (byte >= 0xf8) {
      send(byte); // real time messages can arrive anywhere
      return;
    }
    if (byte === 0xf0) {
      sysex = [byte];
      return;
    }
    if (sysex !== null) {
      sysex.push(byte);
      if (byte === 0xf7) {
        sendAll(sysex);
        sysex = null;
      }
      return;
    }
    if (byte >= 0x80) {
      status = byte;
      data = [];
    } else if (status === 0) {
      return; // data byte with no status, ignore
    } else {
      data.push(byte);
    }
    if (data.length === dataLength(status)) {
      handleMessage([status].concat(data));
      data = [];
      if (status >= 0xf0) status = 0; // no running status for system messages
    }
  }

  return {
    input: input,
    activeNotes: function () {
      return router.activeNotes();
    },
    setOutput: setOutput,
    setMode: engine.setMode,
    setStrength: engine.setStrength,
    setSemitones: engine.setSemitones,
    reset: engine.reset,
    drift: engine.drift,
  };
}

// Max glue. outlet() only exists inside Max.
var tap = createTap(function (byte) {
  outlet(0, byte);
}, sendDrift);

// The drift view keeps the last table it was sent, even while its window is closed.
function sendDrift() {
  outlet(1, tap.drift());
}

function msg_int(byte) {
  tap.input(byte);
}

// Messages from the panel. Each live.* control sends its value with a prepend.
function mode(value) {
  tap.setMode(value);
}

function strength(value) {
  tap.setStrength(value);
}

function semitones(value) {
  tap.setSemitones(value);
}

function output(value) {
  tap.setOutput(value);
}

// The Reset button may send 1 then 0, or a bang. Only the press resets.
function reset(value) {
  if (value === 0) return;
  tap.reset();
}

// Reads every setting from the panel. Sent by live.thisdevice once the device has loaded,
// and run here too so a reload from autowatch picks the settings up again.
function sync() {
  var controls = [
    ["mode", mode],
    ["strength", strength],
    ["semitones", semitones],
    ["output", output],
  ];
  for (var i = 0; i < controls.length; i++) {
    var control = this.patcher.getnamed(controls[i][0]);
    if (control) controls[i][1](control.getvalueof());
  }
  sendDrift();
}

if (this.patcher) sync();

if (typeof module !== "undefined" && module.exports) {
  module.exports = {
    createEngine: createEngine,
    createRouter: createRouter,
    createSemitoneRouter: createSemitoneRouter,
    createTap: createTap,
    bendForCents: bendForCents,
    hertzForPitch: hertzForPitch,
    pitchForHertz: pitchForHertz,
  };
}
