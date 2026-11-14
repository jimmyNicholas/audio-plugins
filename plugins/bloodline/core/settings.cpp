#include "settings.h"
#include <algorithm>
#include <cmath>

namespace bloodline {

namespace {

// From #43's starting table. Body / Noise is the share of noise; Decay is in seconds here.
const SettingInfo kInfo[kNumSettings] = {
  { "Pitch", 30.0,  1000.0,  true  },
  { "Sweep", 0.0,   48.0,    false },
  { "Decay", 0.020, 1.5,     true  },
  { "Body / Noise",  0.0,   1.0,     false },
  { "Noise tone", 200.0, 12000.0, true  },
  { "Metal",   0.0,   1.0,     false },
  { "Click",   0.0,   1.0,     false },
  { "Drive",   0.0,   1.0,     false },
};

// Real-unit zones: { lo, hi } per setting, in kInfo's order. Drive is full range for every drum.
struct RealRange { double lo, hi; };
const RealRange kZones[3][kNumSettings] = {
  // Kick
  { { 40, 90 },   { 3, 24 }, { 0.150, 0.600 }, { 0.00, 0.25 }, { 200, 2000 },   { 0.0, 0.05 }, { 0.1, 0.8 }, { 0, 1 } },
  // Snare
  { { 150, 280 }, { 0, 8 },  { 0.080, 0.350 }, { 0.40, 0.80 }, { 1500, 6000 },  { 0.0, 0.2 },  { 0.0, 0.6 }, { 0, 1 } },
  // Hats
  { { 100, 400 }, { 0, 2 },  { 0.030, 0.800 }, { 0.85, 1.00 }, { 5000, 12000 }, { 0.4, 1.0 },  { 0.0, 0.4 }, { 0, 1 } },
};

} // namespace

const SettingInfo& info(int setting) { return kInfo[setting]; }

double toReal(int setting, double norm) {
  const SettingInfo& s = kInfo[setting];
  norm = std::clamp(norm, 0.0, 1.0);
  if (s.logScale) return s.min * std::pow(s.max / s.min, norm);
  return s.min + (s.max - s.min) * norm;
}

double toNorm(int setting, double real) {
  const SettingInfo& s = kInfo[setting];
  real = std::clamp(real, s.min, s.max);
  if (s.logScale) return std::log(real / s.min) / std::log(s.max / s.min);
  return (real - s.min) / (s.max - s.min);
}

Values toReal(const Values& norm) {
  Values out{};
  for (int i = 0; i < kNumSettings; ++i) out[i] = toReal(i, norm[i]);
  return out;
}

Range zoneRange(Zone zone, int setting) {
  if (zone == Zone::None) return { 0.0, 1.0 };
  const RealRange& r = kZones[static_cast<int>(zone) - 1][setting];
  return { toNorm(setting, r.lo), toNorm(setting, r.hi) };
}

const char* zoneName(Zone zone) {
  switch (zone) {
    case Zone::Kick: return "Kick";
    case Zone::Snare: return "Snare";
    case Zone::Hats: return "Hats";
    default: return "None";
  }
}

} // namespace bloodline
