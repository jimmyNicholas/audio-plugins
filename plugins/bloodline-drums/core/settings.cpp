#include "settings.h"
#include <algorithm>
#include <cmath>

namespace bloodline {

namespace {

// From #45's list. Times are in seconds, filters in Hz, Level in dB, Pan from -1 (left) to 1.
const SettingInfo kInfo[kNumSettings] = {
  { "Pitch",        "Hz", 20.0,    2000.0,  true,  true },
  { "Sweep",        "st", 0.0,     60.0,    false, true },
  { "Sweep time",   "s",  0.002,   0.300,   true,  true },
  { "Attack",       "s",  0.0,     0.050,   false, true },
  { "Decay",        "s",  0.005,   4.0,     true,  true },
  { "Body / Noise", "",   0.0,     1.0,     false, true },
  { "Noise tone",   "Hz", 100.0,   16000.0, true,  true },
  { "Noise decay",  "s",  0.005,   4.0,     true,  true },
  { "Metal",        "",   0.0,     1.0,     false, true },
  { "Click",        "",   0.0,     1.0,     false, true },
  { "Click tone",   "Hz", 500.0,   12000.0, true,  true },
  { "Drive",        "",   0.0,     1.0,     false, true },
  { "Bits",         "",   16.0,    2.0,     false, true },
  { "Rate",         "Hz", 48000.0, 1000.0,  true,  true },
  { "High-pass",    "Hz", 20.0,    8000.0,  true,  true },
  { "Low-pass",     "Hz", 200.0,   20000.0, true,  true },
  { "Resonance",    "",   0.0,     0.95,    false, true },
  { "Pan",          "",   -1.0,    1.0,     false, true },
  { "Level",        "dB", -24.0,   0.0,     false, false },
};

const Group kGroups[kNumGroups] = {
  { "Body", Pitch, 5 }, { "Noise", BodyNoise, 4 }, { "Click", Click, 2 }, { "Grit", Drive, 3 }, { "Out", HighPass, 5 },
};

// Real-unit zones for the eleven zoned settings (Pitch to Click tone), in kInfo's order. The new
// settings' numbers (Sweep time, Attack, Noise decay, Click tone) are a first draft (#45), to tune
// by ear in the second spike.
struct RealRange { double lo, hi; };
constexpr int kZoned = ClickTone + 1;
const RealRange kZones[3][kZoned] = {
  // Kick
  { { 40, 90 }, { 3, 24 }, { 0.015, 0.080 }, { 0, 0.003 }, { 0.150, 0.600 },
    { 0.00, 0.25 }, { 200, 2000 }, { 0.010, 0.080 }, { 0.0, 0.05 },
    { 0.1, 0.8 }, { 1000, 5000 } },
  // Snare
  { { 150, 280 }, { 0, 8 }, { 0.010, 0.050 }, { 0, 0.002 }, { 0.080, 0.350 },
    { 0.40, 0.80 }, { 1500, 6000 }, { 0.100, 0.400 }, { 0.0, 0.2 },
    { 0.0, 0.6 }, { 2000, 8000 } },
  // Hats
  { { 100, 400 }, { 0, 2 }, { 0.002, 0.020 }, { 0, 0.001 }, { 0.030, 0.800 },
    { 0.85, 1.00 }, { 5000, 16000 }, { 0.030, 0.800 }, { 0.4, 1.0 },
    { 0.0, 0.4 }, { 4000, 12000 } },
};

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

Values toReal(const Values& norm) {
  Values out{};
  for (int i = 0; i < kNumSettings; ++i) out[i] = toReal(i, norm[i]);
  return out;
}

Range zoneRange(Zone zone, int setting) {
  if (zone == Zone::None || setting >= kZoned) return { 0.0, 1.0 };
  const RealRange& r = kZones[static_cast<int>(zone) - 1][setting];
  const double a = toNorm(setting, r.lo), b = toNorm(setting, r.hi);
  return { std::min(a, b), std::max(a, b) };
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
