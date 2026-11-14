// The shared drum voice's eight settings, their full ranges, and the Kick, Snare and Hats zones.
// Plain C++17, no JUCE. Breeding works on normalised values (0 to 1); the voice reads real ones.
#pragma once
#include <array>

namespace bloodline {

constexpr int kNumSettings = 8;
using Values = std::array<double, kNumSettings>; // normalised, 0 to 1

enum Setting { Pitch, Sweep, Decay, BodyNoise, NoiseTone, Metal, Click, Drive };

// None breeds anywhere; the others are zones that centre breeding (#43).
enum class Zone { None, Kick, Snare, Hats };
constexpr int kNumZones = 4;

struct SettingInfo {
  const char* id;     // short, stable: used in saved state
  const char* name;   // shown in the UI
  const char* unit;   // "Hz", "st", "ms", "%" or ""
  double min, max;    // full range, in real units
  bool logScale;
};

const SettingInfo& info(int setting);

// Normalised to real units and back.
double toReal(int setting, double norm);
double toNorm(int setting, double real);
Values toReal(const Values& norm);

// A zone's range on one setting, normalised. Zone::None spans 0 to 1.
struct Range { double lo, hi; };
Range zoneRange(Zone zone, int setting);

const char* zoneName(Zone zone);

} // namespace bloodline
