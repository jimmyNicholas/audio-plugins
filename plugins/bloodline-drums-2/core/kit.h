// PROTOTYPE (throwaway spike). Bloodline Drums 2: Bloodline Synth's litter of eight, breeding
// Bloodline Drums' 19-setting voice, with eight keep pads where the synth has its keyboard. Drag a
// candidate onto a pad to keep it; the litter then starts fresh, so the next drum can be bred.
// Drag a kept pad back onto the litter to breed from it again. Plain C++17, no JUCE.
#pragma once
#include "../../shared/breed/litter.h"
#include "settings.h"
#include <array>
#include <string>
#include <vector>

namespace drums2 {

using bloodline::Values;
using bloodline::Zone;

// What the litter calls to breed drum sounds: Bloodline Drums' rules (zones centre fresh draws and
// pull mutations back softly, locks copy, Level is never bred) and Bloodline Synth's cross (whole
// groups from one parent or the other, sometimes blended).
struct DrumOps {
  Zone zone = Zone::None;
  std::array<bool, bloodline::kNumSettings> locks{};
  double zoneShare = 0.8, pull = 0.5;

  Values fresh(breed::Rng& rng) const;
  Values mutate(const Values& from, double spread, breed::Rng& rng) const;
  Values cross(const Values& a, const Values& b, breed::Rng& rng) const;
};

struct Pad {
  bool full = false;
  Values sound{};
  std::string name; // "Kick 2": the zone it was bred in, and how many were kept before it
};

class Kit {
public:
  static constexpr int kLitter = 8, kPads = 8;
  static constexpr int kFirstPad = 36;  // 36 to 43: the keep pads (Bank A's bottom two rows)
  static constexpr int kFirstTile = 44; // 44 to 51: the litter, A to H (the top two rows)

  explicit Kit(std::uint32_t seed = 1);

  const breed::Litter<Values>& litter() const { return litter_; }
  const Values& heard() const { return litter_.heard(); }
  const Pad& pad(int p) const { return pads_[(size_t) p]; }
  Zone zone() const { return ops_.zone; }
  bool locked(int s) const { return ops_.locks[(size_t) s]; }

  void select(int i) { litter_.select(i); }
  void breedFrom(int i) { litter_.breed(i, ops_); }
  void cross(int i, int j) { litter_.cross(i, j, ops_); }
  void fresh() { litter_.refresh(ops_); }
  bool back() { return litter_.back(); }
  void setZone(Zone z) { ops_.zone = z; }
  void setLock(int s, bool on) { if (s >= 0 && s < bloodline::kNumSettings) ops_.locks[(size_t) s] = on; }
  void setValue(int s, double norm);

  // Keeps candidate `tile` on pad `p` (replacing what was there), then draws a fresh litter.
  // Back brings the old litter back; the pad stays kept.
  void keep(int tile, int p);
  void swapPads(int a, int b);
  void clearPad(int p);
  // The pad's sound replaces the heard candidate, which then becomes the parent of a new litter.
  void breedFromPad(int p);

  std::string save() const;
  bool load(const std::string& text); // false, changing nothing, if it is not a saved state

private:
  DrumOps ops_;
  breed::Litter<Values> litter_;
  std::array<Pad, kPads> pads_{};
  int kept_ = 0;
};

// A loop to hear the heard candidate in time while you click through the litter.
class Loop {
public:
  static constexpr int kPatterns = 3;
  static const char* patternName(int p);
  struct Hit { int offset; float velocity; };

  void prepare(double sampleRate) { sr_ = sampleRate; pos_ = 0; }
  void setOn(bool on) { if (on && !on_) pos_ = 0; on_ = on; }
  bool on() const { return on_; }
  void setPattern(int p) { pattern_ = p < 0 || p >= kPatterns ? 0 : p; }
  int pattern() const { return pattern_; }
  // Advances n samples at `bpm`, appending the hits that fall in the block.
  void advance(int n, double bpm, std::vector<Hit>& out);

private:
  double sr_ = 48000.0, pos_ = 0.0; // pos_ in sixteenths
  int pattern_ = 1;
  bool on_ = false;
};

} // namespace drums2
