// PROTOTYPE (throwaway spike). Bloodline Breaks 2's left panel: Bloodline Drums 2's breeding, cut
// to a litter of four, with eight keep pads. Each pad remembers the zone it was bred in, so the
// right panel knows which pads are kicks, snares, hats or anything else. Plain C++17, no JUCE.
#pragma once
#include "../../shared/breed/litter.h"
#include "settings.h"
#include <array>
#include <iosfwd>
#include <string>
#include <vector>

namespace bb2 {

using bloodline::Values;
using bloodline::Zone;

// Bloodline Drums 2's rules, as they were: zones centre fresh draws and pull mutations back softly,
// locks copy, Level is never bred; cross takes whole groups from one parent or the other.
struct DrumOps {
  Zone zone = Zone::Kick;
  std::array<bool, bloodline::kNumSettings> locks{};
  double zoneShare = 0.8, pull = 0.5;

  Values fresh(breed::Rng& rng) const;
  Values mutate(const Values& from, double spread, breed::Rng& rng) const;
  Values cross(const Values& a, const Values& b, breed::Rng& rng) const;
};

struct Pad {
  bool full = false;
  Values sound{};
  Zone zone = Zone::None; // the label: which lane of a loop it belongs to
  std::string name;       // "Kick 2"
};

class Drums {
public:
  static constexpr int kLitter = 4, kPads = 8;

  explicit Drums(std::uint32_t seed = 1);

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

  // Keeps candidate `tile` on pad `p` (replacing what was there), labelled with the zone, then
  // draws a fresh litter. Back brings the old litter back; the pad stays kept.
  void keep(int tile, int p);
  void swapPads(int a, int b);
  void clearPad(int p);
  void breedFromPad(int p); // the pad's sound becomes the parent of a new litter

  void save(std::ostream&) const;
  bool load(std::istream&); // false, changing nothing, if it is not a saved state

private:
  DrumOps ops_;
  breed::Litter<Values> litter_;
  std::array<Pad, kPads> pads_{};
  int kept_ = 0;
};

// A rhythm to hear the heard drum in time. In break plays it in the right panel's loop instead,
// on its own lane (Kick on the kick lane, and so on; None on the top lane).
class Audition {
public:
  enum Pattern { Quarters, BoomBap, Sixteenths, InBreak, kPatterns };
  static const char* patternName(int p);
  struct Hit { int offset; float velocity; };

  void prepare(double sampleRate) { sr_ = sampleRate; pos_ = 0; }
  void setOn(bool on) { if (on && !on_) pos_ = 0; on_ = on; }
  bool on() const { return on_; }
  void setPattern(int p) { pattern_ = p < 0 || p >= kPatterns ? 0 : p; }
  int pattern() const { return pattern_; }
  bool inBreak() const { return on_ && pattern_ == InBreak; }
  void advance(int n, double bpm, std::vector<Hit>& out); // nothing In break

private:
  double sr_ = 48000.0, pos_ = 0.0; // pos_ in sixteenths
  int pattern_ = InBreak;
  bool on_ = false;
};

} // namespace bb2
