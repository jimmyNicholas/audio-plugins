// Snare Breeder spike: the snare voice. ES5 (Max 8's js object): var, function, no arrows, no
// Math.tanh or Math.imul. `npm run build` joins this with the shared breeding engine and Max
// glue (shared/breeder/) into snare-breeder.js, which is what the device loads.
//
// A snare is two sounds at once: the drum skin (two swept sines, a fundamental and one
// overtone) and the snare wires (band-passed noise). Bursts restarts the wires a few times
// before the tail, which turns a snare into a clap. Snap is a short, bright crack on top.
//
// Throwaway spike: see README.md in this folder for what it is testing.

var Drum = (function () {
  var PARAMS = [
    { id: "pitch", name: "Pitch", short: "Pitch", min: 120, max: 400, scale: "exp" },
    { id: "sweep", name: "Sweep", short: "Sweep", min: 0, max: 24, scale: "lin" },
    { id: "sweepTime", name: "Sweep time", short: "Sweep T", min: 5, max: 80, scale: "exp" },
    { id: "ratio", name: "Overtone ratio", short: "Ratio", min: 1.3, max: 2.4, scale: "lin" },
    { id: "ring", name: "Overtone level", short: "Ring", min: 0, max: 1, scale: "lin" },
    { id: "bodyDecay", name: "Body decay", short: "Body D", min: 30, max: 500, scale: "exp" },
    { id: "balance", name: "Body to wires", short: "Balance", min: 0, max: 1, scale: "lin" },
    { id: "wiresDecay", name: "Wires decay", short: "Wires D", min: 40, max: 800, scale: "exp" },
    { id: "wiresTone", name: "Wires tone", short: "Wires T", min: 800, max: 10000, scale: "exp" },
    { id: "width", name: "Wires width", short: "Width", min: 0, max: 1, scale: "lin" },
    { id: "bursts", name: "Bursts", short: "Bursts", min: 0, max: 1, scale: "lin" },
    { id: "spread", name: "Burst spread", short: "Spread", min: 4, max: 20, scale: "exp" },
    { id: "snap", name: "Snap", short: "Snap", min: 0, max: 1, scale: "lin" },
    { id: "compression", name: "Compression", short: "Comp", min: 0, max: 1, scale: "lin" },
    { id: "saturation", name: "Saturation", short: "Sat", min: 0, max: 1, scale: "lin" },
    { id: "tone", name: "Tone", short: "Tone", min: 1000, max: 16000, scale: "exp" }
  ];
  // What the patch stores per slot, in the order its trigger fires them: settings first.
  var PARTS = ["statics", "freq", "wires", "amp", "snap"];
  var TARGET_RMS = 0.2, RAW_GAIN = 0.5, PEAK_LIMIT = 0.95;
  var SNAP_FREQ = 2500, SNAP_RES = 0.2; // fixed in the patch: svf~ 2500 0.2, high-pass out
  var D = BreederDsp, tanh = D.tanh;

  function real(values) { return D.realValues(PARAMS, values); }

  // Half of all snares have one burst (a plain snare); the rest have 2, 3 or 4 (towards a clap).
  function burstCount(v) { return v < 0.5 ? 1 : 2 + Math.min(2, Math.floor((v - 0.5) * 6)); }
  // Equal-power balance: 0 is all body, 1 all wires.
  function bodyLevel(p) { return Math.cos(p.balance * Math.PI / 2); }
  function wiresLevel(p) { return Math.sin(p.balance * Math.PI / 2); }
  // svf~ resonance for the wires: wide (0) to narrow and ringing (0.8).
  function wiresRes(p) { return 0.8 * (1 - p.width); }

  function freqPoints(p) {
    var start = Math.min(p.pitch * Math.pow(2, p.sweep / 12), 16000);
    return D.sweepPoints(start, p.pitch, p.sweepTime, 3);
  }
  function ampPoints(p) { return D.decayPoints(bodyLevel(p), p.bodyDecay, p.compression); }
  // Each early burst jumps up and falls away over Spread, then the last one rings out.
  function wiresPoints(p) {
    var n = burstCount(p.bursts), peak = wiresLevel(p), tail = D.decayPoints(peak, p.wiresDecay, p.compression);
    var out = [0, 0], i;
    if (n === 1) return tail;
    for (i = 1; i < n; i++) out.push(peak, 0.5, peak * 0.12, p.spread - 0.5);
    return out.concat(tail.slice(2));
  }
  function snapPoints(p) { return D.clickPoints(p.snap * 1.5); }

  // A rough simulation of the patch's voice, to measure loudness: the loudest 50 ms window of
  // weighted RMS in the first 300 ms. Close enough to level-match snares; not sample exact.
  function loudness(p) {
    var sr = 22050, len = Math.floor(sr * 0.3), msPer = 1000 / sr;
    var f = D.stepper(freqPoints(p), msPer), a = D.stepper(ampPoints(p), msPer);
    var w = D.stepper(wiresPoints(p), msPer), sn = D.stepper(snapPoints(p), msPer);
    var k = 1 + p.saturation * 12, outGain = 1 / tanh(k), white = D.noise(12345);
    var wires = D.svf(p.wiresTone, wiresRes(p), sr), snap = D.svf(SNAP_FREQ, SNAP_RES, sr);
    var weigh = D.weighting(sr), lp = 1 - Math.exp(-2 * Math.PI * Math.min(p.tone, sr / 2.2) / sr);
    var ph1 = 0.75, ph2 = 0.75, y = 0, heard = [], raw = [], i, n, x, hz;
    for (i = 0; i < len; i++) {
      hz = f();
      ph1 += hz / sr;
      ph2 += hz * p.ratio / sr;
      n = white();
      wires(n);
      snap(n);
      x = (Math.cos(2 * Math.PI * ph1) + p.ring * Math.cos(2 * Math.PI * ph2)) * a() + wires.band * w() + snap.high * sn();
      x = tanh(k * x) * outGain;
      y += lp * (x - y);
      raw.push(y);
      heard.push(weigh(y));
    }
    return { rms: D.measure(heard, sr).rms, peak: D.measure(raw, sr).peak };
  }

  // Everything the patch needs to play one snare. match: level-match it to the others.
  function voice(values, match) {
    var p = real(values), k = 1 + p.saturation * 12;
    var gain = match ? D.matchGain(loudness(p), TARGET_RMS, PEAK_LIMIT) : RAW_GAIN;
    return {
      freq: freqPoints(p), amp: ampPoints(p), wires: wiresPoints(p), snap: snapPoints(p),
      // In the order the patch unpacks them: overtone ratio and level, wires tone and
      // resonance, saturation in and out, tone, output gain.
      statics: [p.ratio, p.ring, p.wiresTone, wiresRes(p), k, 1 / tanh(k), p.tone, gain]
    };
  }

  function describe(values) {
    var p = real(values), hz = D.hz, n = burstCount(p.bursts);
    return hz(p.pitch) + ", ring " + p.ring.toFixed(2) + " at " + p.ratio.toFixed(2) + "x, body " +
      p.bodyDecay.toFixed(0) + " ms, wires " + p.wiresDecay.toFixed(0) + " ms at " + hz(p.wiresTone) +
      ", balance " + p.balance.toFixed(2) + ", " + n + (n === 1 ? " burst" : " bursts") +
      ", snap " + p.snap.toFixed(2) + ", comp " + p.compression.toFixed(2) + ", sat " + p.saturation.toFixed(2) +
      ", tone " + hz(p.tone);
  }
  // A short name for a saved slot.
  function label(values) {
    var p = real(values);
    return D.hz(p.pitch) + (burstCount(p.bursts) >= 3 ? " clap" : " snare");
  }

  return {
    NOUN: "snare", PARAMS: PARAMS, PARTS: PARTS, SNAP_FREQ: SNAP_FREQ, SNAP_RES: SNAP_RES,
    voice: voice, loudness: loudness, describe: describe, label: label, burstCount: burstCount
  };
})();
