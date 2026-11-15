// PROTOTYPE (throwaway spike). Bloodline Synth 2's left panel: Bloodline Synth's patches, bred in a
// litter of four, with eight keep pads. Each pad remembers the zone it was bred in, so the right
// panel knows which pads are basses, keys, leads, pads or plucks. Plain C++17, no JUCE.
#pragma once
#include "patch.h"
#include <array>
#include <iosfwd>
#include <string>

namespace bs2 {

using synth::Patch;
using synth::Zone;

struct Pad {
  bool full = false;
  Patch sound{};
  Zone zone = Zone::None; // the label the loops choose by
  std::string name;       // "Bass 2", or "Synth 3" for None
};

const char* padLabel(Zone z);

class Sounds {
public:
  static constexpr int kLitter = 4, kPads = 8;

  explicit Sounds(std::uint32_t seed = 1);

  const breed::Litter<Patch>& litter() const { return litter_; }
  const Patch& heard() const { return litter_.heard(); }
  const Pad& pad(int p) const { return pads_[(size_t) p]; }
  Zone zone() const { return ops_.zone; }
  bool locked(int s) const { return ops_.locks[(size_t) s]; }
  bool anyKept() const;

  void select(int i) { litter_.select(i); }
  void breedFrom(int i) { litter_.breed(i, ops_); }
  void cross(int i, int j) { litter_.cross(i, j, ops_); }
  void fresh() { litter_.refresh(ops_); }
  bool back() { return litter_.back(); }
  void setSpread(double s) { litter_.setSpread(s); }
  void setZone(Zone z) { ops_.zone = z; }
  void setLock(int s, bool on) { if (s >= 0 && s < synth::kNumSettings) ops_.locks[(size_t) s] = on; }
  void setValue(int s, double norm); // on the heard candidate

  // Keeps candidate `tile` on pad `p`, labelled with the zone. The litter stays as it is, so the
  // same tile or a sibling can be kept again.
  void keep(int tile, int p);
  void swapPads(int a, int b);
  void clearPad(int p);
  void breedFromPad(int p); // the pad's sound becomes the parent of a new litter

  void save(std::ostream&) const;
  bool load(std::istream&); // false, changing nothing, if it is not a saved state

private:
  synth::PatchOps ops_;
  breed::Litter<Patch> litter_;
  std::array<Pad, kPads> pads_{};
  int kept_ = 0;
};

} // namespace bs2
