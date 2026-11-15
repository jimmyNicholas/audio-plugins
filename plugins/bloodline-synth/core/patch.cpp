#include "patch.h"
#include <algorithm>
#include <cmath>

namespace synth {

namespace {

// Wide on purpose: experimental ranges, well outside the usual norms.
const SettingInfo kInfo[kNumSettings] = {
  { "Shape A",      "",    0.0,    1.0,     false, true },
  { "Shape B",      "",    0.0,    1.0,     false, true },
  { "Tune B",       "st",  -24.0,  24.0,    false, true },
  { "Detune",       "ct",  0.0,    60.0,    false, true },
  { "Mix B",        "",    0.0,    1.0,     false, true },
  { "Sub",          "",    0.0,    1.0,     false, true },
  { "Noise",        "",    0.0,    1.0,     false, true },
  { "FM",           "",    0.0,    1.0,     false, true },
  { "Cutoff",       "Hz",  30.0,   18000.0, true,  true },
  { "Resonance",    "",    0.0,    0.97,    false, true },
  { "Env amount",   "st",  -48.0,  72.0,    false, true },
  { "Filter decay", "s",   0.005,  3.0,     true,  true },
  { "Drive",        "",    0.0,    1.0,     false, true },
  { "Attack",       "s",   0.001,  2.0,     true,  true },
  { "Decay",        "s",   0.01,   4.0,     true,  true },
  { "Sustain",      "",    0.0,    1.0,     false, true },
  { "Release",      "s",   0.005,  4.0,     true,  true },
  { "LFO rate",     "Hz",  0.05,   30.0,    true,  true },
  { "Vibrato",      "st",  0.0,    2.0,     false, true },
  { "Wobble",       "oct", 0.0,    4.0,     false, true },
  { "Glide",        "s",   0.0005, 1.0,     true,  true },
  { "Bits",         "bits", 16.0,  3.0,     false, true },
  { "Width",        "",    0.0,    1.0,     false, true },
  { "Level",        "dB",  -24.0,  0.0,     false, false },
};

const Group kGroups[kNumGroups] = {
  { "Osc", ShapeA, 8 }, { "Filter", Cutoff, 5 }, { "Amp", Attack, 4 }, { "Motion", LfoRate, 4 }, { "Out", Bits, 3 },
};

// A first guess at each zone, in real units. Settings not listed span their full range.
struct ZoneEntry { int setting; double lo, hi; };
const ZoneEntry kBass[] = {
  { ShapeA, 0.3, 1 }, { TuneB, -12, 0 }, { Sub, 0.3, 1 }, { Noise, 0, 0.1 }, { FM, 0, 0.3 },
  { Cutoff, 80, 1500 }, { Resonance, 0, 0.6 }, { EnvAmount, 0, 36 }, { FilterDecay, 0.05, 0.5 },
  { Attack, 0.001, 0.01 }, { Decay, 0.1, 1 }, { Sustain, 0.3, 1 }, { Release, 0.01, 0.2 },
  { Vibrato, 0, 0.1 }, { Wobble, 0, 0.5 }, { Glide, 0.0005, 0.12 }, { Width, 0, 0.2 }, { -1, 0, 0 },
};
const ZoneEntry kKeys[] = {
  { Sub, 0, 0.3 }, { Noise, 0, 0.1 }, { Cutoff, 800, 6000 }, { EnvAmount, 0, 24 }, { FilterDecay, 0.1, 1 },
  { Attack, 0.001, 0.02 }, { Decay, 0.3, 2 }, { Sustain, 0, 0.6 }, { Release, 0.1, 0.8 },
  { Vibrato, 0, 0.2 }, { Wobble, 0, 0.3 }, { Glide, 0.0005, 0.01 }, { Width, 0.2, 0.7 }, { -1, 0, 0 },
};
const ZoneEntry kLead[] = {
  { ShapeA, 0.5, 1 }, { Sub, 0, 0.3 }, { Cutoff, 1000, 8000 }, { Resonance, 0.2, 0.8 },
  { Attack, 0.001, 0.05 }, { Decay, 0.2, 1 }, { Sustain, 0.5, 1 }, { Release, 0.05, 0.4 },
  { Vibrato, 0.1, 0.6 }, { Glide, 0.01, 0.2 }, { Width, 0, 0.4 }, { -1, 0, 0 },
};
const ZoneEntry kPad[] = {
  { Detune, 10, 40 }, { Noise, 0, 0.2 }, { FM, 0, 0.2 }, { Cutoff, 300, 4000 }, { Resonance, 0, 0.5 },
  { Attack, 0.2, 2 }, { Decay, 1, 4 }, { Sustain, 0.5, 1 }, { Release, 0.8, 4 },
  { LfoRate, 0.05, 1 }, { Wobble, 0.1, 1 }, { Glide, 0.0005, 0.05 }, { Width, 0.5, 1 }, { -1, 0, 0 },
};
const ZoneEntry kPluck[] = {
  { Cutoff, 300, 3000 }, { Resonance, 0.1, 0.7 }, { EnvAmount, 24, 60 }, { FilterDecay, 0.02, 0.3 },
  { Attack, 0.001, 0.005 }, { Decay, 0.05, 0.5 }, { Sustain, 0, 0.1 }, { Release, 0.05, 0.4 }, { -1, 0, 0 },
};
const ZoneEntry* const kZones[kNumZones - 1] = { kBass, kKeys, kLead, kPad, kPluck };

} // namespace

const Group& group(int g) { return kGroups[g]; }
const SettingInfo& info(int setting) { return kInfo[setting]; }

double toReal(int setting, double norm) {
  const SettingInfo& s = kInfo[setting];
  norm = std::clamp(norm, 0.0, 1.0);
  if (s.logScale) return s.min * std::pow(s.max / s.min, norm);
  return s.min + (s.max - s.min) * norm;
}

double toNorm(int setting, double real) {
  const SettingInfo& s = kInfo[setting];
  real = std::clamp(real, std::min(s.min, s.max), std::max(s.min, s.max));
  if (s.logScale) return std::log(real / s.min) / std::log(s.max / s.min);
  return (real - s.min) / (s.max - s.min);
}

const char* zoneName(Zone z) {
  switch (z) {
    case Zone::Bass: return "Bass";
    case Zone::Keys: return "Keys";
    case Zone::Lead: return "Lead";
    case Zone::Pad: return "Pad";
    case Zone::Pluck: return "Pluck";
    default: return "None";
  }
}

Range zoneRange(Zone z, int setting) {
  if (z == Zone::None) return { 0, 1 };
  for (const ZoneEntry* e = kZones[static_cast<int>(z) - 1]; e->setting >= 0; ++e)
    if (e->setting == setting) {
      const double a = toNorm(setting, e->lo), b = toNorm(setting, e->hi);
      return { std::min(a, b), std::max(a, b) };
    }
  return { 0, 1 };
}

Patch PatchOps::fresh(breed::Rng& rng) const {
  Patch p{};
  for (int i = 0; i < kNumSettings; ++i) {
    Range r{ 0, 1 };
    if (zone != Zone::None && rng.uniform() < zoneShare) r = zoneRange(zone, i);
    p[i] = r.lo + (r.hi - r.lo) * rng.uniform();
  }
  p[Level] = toNorm(Level, -6.0); // a little headroom; never bred
  return p;
}

Patch PatchOps::mutate(const Patch& from, double spread, breed::Rng& rng) const {
  Patch p{};
  for (int i = 0; i < kNumSettings; ++i) {
    if (!info(i).bred || locks[i]) { p[i] = from[i]; continue; }
    double x = breed::fold(from[i] + rng.gauss() * spread);
    if (zone != Zone::None) {
      const Range r = zoneRange(zone, i);
      if (x < r.lo) x += (r.lo - x) * pull;
      else if (x > r.hi) x -= (x - r.hi) * pull;
    }
    p[i] = x;
  }
  return p;
}

// Whole groups pass down together, so a child gets one parent's filter or the other's, which
// sounds more like either parent than a setting-by-setting shuffle. One group in five is a blend.
Patch PatchOps::cross(const Patch& a, const Patch& b, breed::Rng& rng) const {
  Patch p = a;
  for (int g = 0; g < kNumGroups; ++g) {
    const double r = rng.uniform();
    const double mix = r < 0.4 ? 0.0 : r < 0.8 ? 1.0 : rng.uniform();
    for (int i = kGroups[g].first; i < kGroups[g].first + kGroups[g].count; ++i)
      if (!locks[i] && info(i).bred) p[i] = a[i] + (b[i] - a[i]) * mix;
  }
  return p;
}

} // namespace synth
