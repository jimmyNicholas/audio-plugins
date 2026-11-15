// PROTOTYPE (throwaway spike). Everything Bloodline Breaks keeps, without JUCE: a litter of eight
// one-bar patterns, the soft pulls that steer breeding (style, swing dial, pull, taste), each
// lane's role and lock, and the text saved with a set.
#pragma once
#include "pattern.h"
#include <algorithm>
#include <string>

namespace breaks {

class Breeder {
public:
  static constexpr int kLitter = 8;

  explicit Breeder(std::uint32_t seed = 1);

  const breed::Litter<Pattern>& litter() const { return litter_; }
  const Pattern& heard() const { return litter_.heard(); }
  const PatternOps& ops() const { return ops_; }

  void select(int i) { litter_.select(i); }
  // Breeding and crossing teach the taste what you chose.
  void breedFrom(int i);
  void cross(int i, int j);
  void fresh() { litter_.refresh(ops_); }
  bool back() { return litter_.back(); }

  // These steer the next breed or fresh litter; they change no candidate now. Choosing a style
  // also moves the swing dial to that style's usual swing.
  void setStyle(Style s);
  void setSwingDial(double s) { ops_.swingDial = std::clamp(s, kSwingMin, kSwingMax); }
  void setPull(double p) { ops_.pull = std::clamp(p, 0.0, 1.0); }
  void setLock(int lane, bool on) { if (lane >= 0 && lane < kLanes) ops_.locks[(size_t) lane] = on; }
  // A new role forgets that lane's taste. Off empties the lane in every candidate at once.
  void setRole(int lane, Role r);
  void forgetTaste() { ops_.taste.forget(); }

  // Changing the heard candidate by hand. A step goes rest, hit, ghost, rest.
  void cycleStep(int lane, int step);
  void setSwing(double s);

  std::string save() const;
  bool load(const std::string& text); // false, changing nothing, if it is not a saved state

private:
  PatternOps ops_;
  breed::Litter<Pattern> litter_;
};

} // namespace breaks
