// TAP MPE spike (ticket 01). Throwaway: proves per-note pitch bend reaches the instrument.
//
// Reads raw MIDI bytes from [midiin] and writes MPE bytes for [midiout]:
// - each note on gets its own channel, 2 to 16, with a pitch bend sent just before it
// - note off goes out on the note's channel
// - everything else goes out on channel 1, the MPE master channel
//
// Written in ES5 for Max 8's js object. Also loads in Node for the tests.

autowatch = 1;
inlets = 1;
outlets = 1;

var BEND_RANGE_SEMITONES = 48;
var FIRST_MEMBER_CHANNEL = 1; // zero-based, so MIDI channel 2
var LAST_MEMBER_CHANNEL = 15; // MIDI channel 16
var KEYS_BLACK = 0;
var KEYS_ALL = 1;

// 14-bit pitch bend value (8192 is centre) for an offset in cents.
function bendForCents(cents) {
  var value = 8192 + Math.round((cents * 8192) / (BEND_RANGE_SEMITONES * 100));
  return Math.max(0, Math.min(16383, value));
}

function isBlackKey(note) {
  var pitchClass = note % 12;
  return pitchClass === 1 || pitchClass === 3 || pitchClass === 6 || pitchClass === 8 || pitchClass === 10;
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

// send(byte) is called for every output byte, in order.
function createSpike(send) {
  var detuneCents = 0;
  var keys = KEYS_BLACK;

  // Byte parser state.
  var status = 0;
  var data = [];
  var sysex = null;

  // One voice per member channel. note is -1 when free. order is when the note started,
  // or when the channel was freed, so we can steal the oldest and reuse the longest free.
  var voices = [];
  var counter = 0;
  for (var ch = FIRST_MEMBER_CHANNEL; ch <= LAST_MEMBER_CHANNEL; ch++) {
    voices.push({ channel: ch, note: -1, order: 0 });
  }

  function sendAll(bytes) {
    for (var i = 0; i < bytes.length; i++) send(bytes[i]);
  }

  function findVoice(note) {
    for (var i = 0; i < voices.length; i++) {
      if (voices[i].note === note) return voices[i];
    }
    return null;
  }

  function chooseVoice() {
    var best = null;
    var i;
    for (i = 0; i < voices.length; i++) {
      if (voices[i].note === -1 && (best === null || voices[i].order < best.order)) best = voices[i];
    }
    if (best !== null) return best;
    for (i = 0; i < voices.length; i++) {
      if (best === null || voices[i].order < best.order) best = voices[i];
    }
    sendAll([0x80 | best.channel, best.note, 0]);
    return best;
  }

  function noteOff(note, velocity) {
    var voice = findVoice(note);
    if (voice === null) return;
    sendAll([0x80 | voice.channel, note, velocity]);
    voice.note = -1;
    voice.order = ++counter;
  }

  function noteOn(note, velocity) {
    noteOff(note, 0);
    var voice = chooseVoice();
    var cents = keys === KEYS_ALL || isBlackKey(note) ? detuneCents : 0;
    var bend = bendForCents(cents);
    sendAll([0xe0 | voice.channel, bend & 0x7f, bend >> 7]);
    sendAll([0x90 | voice.channel, note, velocity]);
    voice.note = note;
    voice.order = ++counter;
  }

  function handleMessage(bytes) {
    var type = bytes[0] & 0xf0;
    if (type === 0x90 && bytes[2] > 0) {
      noteOn(bytes[1], bytes[2]);
    } else if (type === 0x80 || type === 0x90) {
      noteOff(bytes[1], type === 0x80 ? bytes[2] : 0);
    } else if (type === 0xa0) {
      var voice = findVoice(bytes[1]);
      if (voice !== null) sendAll([0xa0 | voice.channel, bytes[1], bytes[2]]);
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
    setDetune: function (cents) {
      detuneCents = cents;
    },
    setKeys: function (value) {
      keys = value;
    },
  };
}

// Max glue. outlet() only exists inside Max.
var spike = createSpike(function (byte) {
  outlet(0, byte);
});

function msg_int(byte) {
  spike.input(byte);
}

function detune(cents) {
  spike.setDetune(cents);
}

function keys(value) {
  spike.setKeys(value);
}

if (typeof module !== "undefined" && module.exports) {
  module.exports = { createSpike: createSpike, bendForCents: bendForCents, isBlackKey: isBlackKey };
}
