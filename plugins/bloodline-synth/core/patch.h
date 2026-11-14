// Bloodline Synth's patch: 24 settings in six groups, the zones that centre breeding, and how a
// patch is drawn, mutated and crossed for the shared litter. Plain C++17, no JUCE. Breeding works
// on normalised values (0 to 1); the engine reads real ones.
#pragma once
#include "../../shared/breed/litter.h"
#include <array>

namespace synth {

constexpr int kNumSettings = 24;
using Patch = std::array<double, kNumSettings>; // normalised, 0 to 1

enum Setting {
  ShapeA, ShapeB, TuneB, Detune, MixB, Sub, Noise, FM,          // Osc
  Cutoff, Resonance, EnvAmount, FilterDecay, Drive,             // Filter
  Attack, Decay, Sustain, Release,                              // Amp
  LfoRate, Vibrato, Wobble, Glide,                              // Motion
  Bits, Width, Level                                            // Out
};

struct Group { const char* name; int first, count; };
constexpr int kNumGroups = 5;
const Group& group(int g);

struct SettingInfo {
  const char* name;
  const char* unit;   // "Hz", "s", "st", "ct", "dB", "oct", "bits" or "" (shown as a percentage)
  double min, max;    // real units at 0 and at 1
  bool logScale;
  bool bred;          // Level is only turned by hand, as in Bloodline
};
const SettingInfo& info(int setting);
double toReal(int setting, double norm);
double toNorm(int setting, double real);

// Zones, as in Bloodline: none breeds anywhere; the others centre fresh draws (80%) and pull
// mutations halfway back.
enum class Zone { None, Bass, Keys, Lead, Pad, Pluck };
constexpr int kNumZones = 6;
const char* zoneName(Zone z);
struct Range { double lo, hi; };
Range zoneRange(Zone z, int setting);

// What the litter calls to breed patches. Locks copy the parent's value.
struct PatchOps {
  Zone zone = Zone::None;
  std::array<bool, kNumSettings> locks{};
  double zoneShare = 0.8, pull = 0.5;

  Patch fresh(breed::Rng& rng) const;
  Patch mutate(const Patch& from, double spread, breed::Rng& rng) const;
  Patch cross(const Patch& a, const Patch& b, breed::Rng& rng) const; // each group from one parent, or a blend
};

} // namespace synth
