// Breeder devices: maths shared by every drum voice. ES5 (Max 8's js object).
// Joined into each device's js by its build; see shared/breeder/README.md.

var BreederDsp = (function () {
  var MODULUS = 2147483647;
  var COMP_THRESHOLD = -24; // dB below the envelope's peak

  function toReal(param, v) {
    if (param.scale === "exp") return param.min * Math.pow(param.max / param.min, v);
    return param.min + (param.max - param.min) * v;
  }
  // Settings from 0 to 1, in PARAMS order, to real units by id.
  function realValues(params, values) {
    var out = {}, i;
    for (i = 0; i < params.length; i++) out[params[i].id] = toReal(params[i], values[i]);
    return out;
  }

  function tanh(x) {
    if (x > 20) return 1;
    if (x < -20) return -1;
    var e = Math.exp(2 * x);
    return (e - 1) / (e + 1);
  }

  // Compression of a one-shot: a static compressor on the envelope. Above the threshold, level
  // in dB is divided by the ratio; then make-up gain brings the peak back to full. The tail
  // comes up against the hit, as when a drum is compressed. It works on the envelope, not the
  // audio, so it never pumps, and it leaves the click alone (like a slow attack).
  function compress(level, amount) {
    if (level <= 0) return 0;
    var ratio = 1 + amount * 9, t = COMP_THRESHOLD;
    var shape = function (db) { return db > t ? t + (db - t) / ratio : db; };
    var db = 20 * Math.log(level) / Math.LN10;
    return Math.pow(10, (shape(db) - shape(0)) / 20);
  }

  // The patch plays line~ envelopes from breakpoint lists: a start value, then (target, time
  // in ms) pairs.

  // Pitch in Hz: from start down to end over `time`, falling as (1 - x)^power.
  function sweepPoints(start, end, time, power) {
    var n = 12, out = [start, 0], i, x;
    for (i = 1; i <= n; i++) {
      x = i / n;
      out.push(end + (start - end) * Math.pow(1 - x, power), time / n);
    }
    return out;
  }
  // A 1.5 ms rise, then an exponential fall to -60 dB over `length`, compressed, then silence.
  function decayPoints(peak, length, comp) {
    var n = 16, out = [0, 0, peak, 1.5], i;
    for (i = 1; i <= n; i++) out.push(peak * compress(Math.exp(-6.9 * i / n), comp), length / n);
    out.push(0, 5);
    return out;
  }
  // A click: starts at `level` and falls with a 4 ms time constant.
  function clickPoints(level) {
    var times = [2, 4, 8, 12, 20], out = [level, 0], last = 0, i;
    for (i = 0; i < times.length; i++) {
      out.push(level * Math.exp(-times[i] / 4), times[i] - last);
      last = times[i];
    }
    out.push(0, 10);
    return out;
  }
  function valueAt(points, ms) {
    var t = 0, i, v0 = points[0], v1, dt;
    for (i = 2; i < points.length; i += 2) {
      v1 = points[i]; dt = points[i + 1];
      if (ms <= t + dt) return dt > 0 ? v0 + (v1 - v0) * (ms - t) / dt : v1;
      t += dt; v0 = v1;
    }
    return v0;
  }
  // Steps through breakpoints sample by sample, faster than valueAt for long runs.
  function stepper(points, msPerSample) {
    var i = 2, t = 0, v0 = points[0], ms = 0;
    return function () {
      while (i < points.length && ms > t + points[i + 1]) { t += points[i + 1]; v0 = points[i]; i += 2; }
      var out = i >= points.length ? v0 : (points[i + 1] > 0 ? v0 + (points[i] - v0) * (ms - t) / points[i + 1] : points[i]);
      ms += msPerSample;
      return out;
    };
  }

  // White noise from -1 to 1, the same every run, so loudness is repeatable.
  function noise(seed) {
    var s = seed || 12345;
    return function () { s = (s * 16807) % MODULUS; return s / MODULUS * 2 - 1; };
  }

  // A state-variable filter for loudness simulations (the "TPT" form, stable up to Nyquist).
  // res 0 to 1, roughly as svf~'s resonance. Call f(x); then read f.low, f.band and f.high.
  function svf(freq, res, sr) {
    var g = Math.tan(Math.PI * Math.min(freq, sr * 0.45) / sr), k = 2 - 1.96 * Math.min(1, Math.max(0, res));
    var a1 = 1 / (1 + g * (g + k)), a2 = g * a1, a3 = g * a2, ic1 = 0, ic2 = 0;
    var f = function (x) {
      var v3 = x - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
      ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2;
      f.low = v2; f.band = v1; f.high = x - k * v1 - v2;
    };
    f.low = 0; f.band = 0; f.high = 0;
    return f;
  }

  // Ears hear 2 to 5 kHz as louder than the same level lower down, so a bright hit with plain
  // RMS matching comes out too loud. A rough stand-in for K-weighting (as in LUFS): a +4 dB
  // shelf above about 1.5 kHz. Returns a function from sample to weighted sample.
  function weighting(sr) {
    var a = 1 - Math.exp(-2 * Math.PI * 1500 / sr), low = 0;
    return function (x) { low += a * (x - low); return x + 0.58 * (x - low); };
  }

  // The loudest 50 ms window of RMS, and the peak, of a rendered hit.
  function measure(data, sr) {
    var win = Math.floor(sr * 0.05), hop = Math.floor(win / 4), best = 0, peak = 0, i, j, sum;
    for (i = 0; i < data.length; i++) if (Math.abs(data[i]) > peak) peak = Math.abs(data[i]);
    for (i = 0; i + win <= data.length; i += hop) {
      sum = 0;
      for (j = i; j < i + win; j++) sum += data[j] * data[j];
      if (sum > best) best = sum;
    }
    return { rms: Math.sqrt(best / win), peak: peak };
  }

  // The gain that brings a hit to the target loudness, without pushing its peak past the limit.
  function matchGain(level, target, peakLimit) {
    var gain = level.rms > 0 ? target / level.rms : 1;
    if (level.peak * gain > peakLimit) gain = peakLimit / level.peak;
    return gain;
  }

  function hz(v) { return v >= 1000 ? (v / 1000).toFixed(1) + " kHz" : v.toFixed(0) + " Hz"; }

  return {
    MODULUS: MODULUS, toReal: toReal, realValues: realValues, tanh: tanh, compress: compress,
    sweepPoints: sweepPoints, decayPoints: decayPoints, clickPoints: clickPoints,
    valueAt: valueAt, stepper: stepper, noise: noise, svf: svf, weighting: weighting,
    measure: measure, matchGain: matchGain, hz: hz
  };
})();
