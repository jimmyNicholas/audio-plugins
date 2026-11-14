// The shared drum voice's 19 settings (#45), their full ranges, the five groups, and the Kick,
// Snare and Hats zones. Plain C++17, no JUCE. Breeding works on normalised values (0 to 1); the
// voice reads real ones.
#pragma once
#include <array>

namespace bloodline {

constexpr int kNumSettings = 19;
using Values = std::array<double, kNumSettings>; // normalised, 0 to 1

// In signal order: voice, then drive, then crusher, then filters, then pan (#45).
enum Setting {
  Pitch, Sweep, SweepTime, Attack, Decay,      // Body
  BodyNoise, NoiseTone, NoiseDecay, Metal,     // Noise
  Click, ClickTone,                            // Click
  Drive, Bits, Rate,                           // Grit
  HighPass, LowPass, Resonance, Pan, Level     // Out
};

struct Group { const char* name; int first, count; };
constexpr int kNumGroups = 5;
const Group& group(int g);

// None breeds anywhere; the others are zones that centre breeding (#43). Zones centre only Body,
// Noise and Click; Grit, Out and Pan cover their full range in every zone (#45).
enum class Zone { None, Kick, Snare, Hats };
constexpr int kNumZones = 4;

struct SettingInfo {
  const char* name;   // shown in the UI
  const char* unit;   // shown after the value
  double min, max;    // real units at 0 and at 1; Bits and Rate run downwards
  bool logScale;
  bool bred;          // Level is only turned by hand (#45)
};

const SettingInfo& info(int setting);

// Normalised to real units and back.
double toReal(int setting, double norm);
double toNorm(int setting, double real);
Values toReal(const Values& norm);

// A zone's range on one setting, normalised. Zone::None, and settings outside Body, Noise and
// Click, span 0 to 1.
struct Range { double lo, hi; };
Range zoneRange(Zone zone, int setting);

const char* zoneName(Zone zone);

} // namespace bloodline
