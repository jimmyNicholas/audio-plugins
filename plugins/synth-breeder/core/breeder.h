// Everything Synth Breeder keeps, without JUCE: a litter of eight patches, the zone and locks that
// steer breeding, and the text that is saved with a set. The JUCE layer plays the heard candidate
// through the Engine.
#pragma once
#include "patch.h"
#include <string>

namespace synth {

class Breeder {
public:
  static constexpr int kLitter = 8;

  explicit Breeder(std::uint32_t seed = 1);

  const breed::Litter<Patch>& litter() const { return litter_; }
  const Patch& heard() const { return litter_.heard(); }

  void select(int i) { litter_.select(i); }
  void breedFrom(int i) { litter_.breed(i, ops_); }
  void cross(int i, int j) { litter_.cross(i, j, ops_); }
  void fresh() { litter_.refresh(ops_); }
  bool back() { return litter_.back(); }

  // Zone and locks steer the next breed; they change no candidate now.
  void setZone(Zone z) { ops_.zone = z; }
  void setLock(int setting, bool on) { if (setting >= 0 && setting < kNumSettings) ops_.locks[(size_t) setting] = on; }
  // Turning a setting by hand on the heard candidate.
  void setValue(int setting, double norm);

  Zone zone() const { return ops_.zone; }
  bool locked(int setting) const { return ops_.locks[(size_t) setting]; }

  std::string save() const;
  bool load(const std::string& text); // false, changing nothing, if it is not a saved state

private:
  PatchOps ops_;
  breed::Litter<Patch> litter_;
};

} // namespace synth
