// Kick Breeder spike: the kick voice. ES5 (Max 8's js object): var, function, no arrows, no
// Math.tanh or Math.imul. `npm run build` joins this with the shared breeding engine and Max
// glue (shared/breeder/) into kick-breeder.js, which is what the device loads.
//
// Throwaway spike: see README.md in this folder for what it is testing.

var Drum = (function () {
  var PARAMS = [
    { id: "pitch", name: "Pitch", short: "Pitch", min: 30, max: 120, scale: "exp" },
    { id: "sweep", name: "Sweep", short: "Sweep", min: 0, max: 48, scale: "lin" },
    { id: "sweepTime", name: "Sweep time", short: "Sweep T", min: 5, max: 200, scale: "exp" },
    { id: "sweepCurve", name: "Sweep curve", short: "Curve", min: 0, max: 1, scale: "lin" },
    { id: "decay", name: "Decay", short: "Decay", min: 50, max: 1500, scale: "exp" },
    { id: "clickLevel", name: "Click level", short: "Click", min: 0, max: 1, scale: "lin" },
    { id: "clickTone", name: "Click tone", short: "Click T", min: 500, max: 12000, scale: "exp" },
    { id: "noiseLevel", name: "Noise level", short: "Noise", min: 0, max: 1, scale: "lin" },
    { id: "noiseDecay", name: "Noise decay", short: "Noise D", min: 10, max: 1000, scale: "exp" },
    { id: "noiseTone", name: "Noise tone", short: "Noise T", min: 200, max: 12000, scale: "exp" },
    { id: "compression", name: "Compression", short: "Comp", min: 0, max: 1, scale: "lin" },
    { id: "saturation", name: "Saturation", short: "Sat", min: 0, max: 1, scale: "lin" },
    { id: "tone", name: "Tone", short: "Tone", min: 200, max: 16000, scale: "exp" }
  ];
  // What the patch stores per slot, in the order its trigger fires them: settings first.
  var PARTS = ["statics", "freq", "noise", "amp", "click"];
  var TARGET_RMS = 0.25, RAW_GAIN = 0.5, PEAK_LIMIT = 0.95;
  var D = BreederDsp, tanh = D.tanh;

  function real(values) { return D.realValues(PARAMS, values); }

  // Frequency in Hz: from pitch * 2^(sweep/12) down to pitch over Sweep time, falling as
  // (1 - x)^power, where power grows with Sweep curve (a snappier fall).
  function freqPoints(p) {
    var start = Math.min(p.pitch * Math.pow(2, p.sweep / 12), 16000);
    return D.sweepPoints(start, p.pitch, p.sweepTime, 1 + p.sweepCurve * 6);
  }
  function ampPoints(p) { return D.decayPoints(1, p.decay, p.compression); }
  function noisePoints(p) { return D.decayPoints(p.noiseLevel * 0.6, p.noiseDecay, p.compression); }
  // Click: a burst of band-passed noise.
  function clickPoints(p) { return D.clickPoints(p.clickLevel * 2); }

  // A rough simulation of the patch's voice, to measure loudness: the loudest 50 ms window of
  // RMS in the first 300 ms. Close enough to level-match kicks; it is not sample exact.
  function loudness(p) {
    var sr = 22050, len = Math.floor(sr * 0.3), msPer = 1000 / sr;
    var f = D.stepper(freqPoints(p), msPer), a = D.stepper(ampPoints(p), msPer);
    var c = D.stepper(clickPoints(p), msPer), nz = D.stepper(noisePoints(p), msPer);
    var k = 1 + p.saturation * 12, outGain = 1 / tanh(k), white = D.noise(12345);
    var phase = 0.75, low = 0, band = 0, nlow = 0;
    // Chamberlin state-variable band-pass for the click, kept below sr / 8 so it stays stable.
    var fc = 2 * Math.sin(Math.PI * Math.min(p.clickTone, sr / 8) / sr), q = 1.0;
    var nfc = 1 - Math.exp(-2 * Math.PI * Math.min(p.noiseTone, sr / 2.2) / sr);
    var lp = 1 - Math.exp(-2 * Math.PI * Math.min(p.tone, sr / 2.2) / sr), y = 0;
    var data = [], i, n, x;
    for (i = 0; i < len; i++) {
      phase += f() / sr;
      n = white();
      low += fc * band; band += fc * (n - low - q * band);
      nlow += nfc * (n - nlow);
      x = Math.cos(2 * Math.PI * phase) * a() + band * c() + nlow * nz();
      x = tanh(k * x) * outGain;
      y += lp * (x - y);
      data.push(y);
    }
    return D.measure(data, sr);
  }

  // Everything the patch needs to play one kick. match: level-match it to the others.
  function voice(values, match) {
    var p = real(values), k = 1 + p.saturation * 12;
    var gain = match ? D.matchGain(loudness(p), TARGET_RMS, PEAK_LIMIT) : RAW_GAIN;
    return {
      freq: freqPoints(p), amp: ampPoints(p), click: clickPoints(p), noise: noisePoints(p),
      // In the order the patch unpacks them: click tone, noise tone, saturation in and out,
      // tone, output gain.
      statics: [p.clickTone, p.noiseTone, k, 1 / tanh(k), p.tone, gain]
    };
  }

  function describe(values) {
    var p = real(values), hz = D.hz;
    return hz(p.pitch) + ", sweep " + p.sweep.toFixed(0) + " st in " + p.sweepTime.toFixed(0) + " ms, decay " +
      p.decay.toFixed(0) + " ms, click " + p.clickLevel.toFixed(2) + ", noise " + p.noiseLevel.toFixed(2) +
      ", comp " + p.compression.toFixed(2) + ", sat " + p.saturation.toFixed(2) + ", tone " + hz(p.tone);
  }
  // A short name for a saved slot.
  function label(values) { return D.hz(real(values).pitch) + " kick"; }

  return {
    NOUN: "kick", PARAMS: PARAMS, PARTS: PARTS,
    voice: voice, loudness: loudness, describe: describe, label: label
  };
})();
