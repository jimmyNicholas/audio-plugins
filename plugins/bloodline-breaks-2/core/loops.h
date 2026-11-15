// PROTOTYPE (throwaway spike). Bloodline Breaks 2's right panel: Bloodline Breaks' one-bar loops,
// cut to a litter of four, with eight keep slots. The four lanes are fixed: kick, snare, hat and
// perc. Each lane of each loop also carries its sound: which of the left panel's kept pads it
// plays. The breeder chooses them, guided by the pads' labels (a Kick pad for the kick lane, and
// so on), and may layer two on one lane. A lane with no kept pad to play uses a built-in sound.
// Plain C++17, no JUCE.
#pragma once
#include "../../shared/breed/litter.h"
#include <array>
#include <cstdint>
#include <iosfwd>

namespace bb2 {

constexpr int kLanes = 4, kSteps = 16, kKitPads = 8;
constexpr double kSwingMin = 0.5, kSwingMax = 0.75; // MPC style: where the off sixteenth lands in its eighth

// The lanes, in the order they are kept; the editor draws Kick at the bottom.
enum Lane { KickLane, SnareLane, HatLane, PercLane };

// Soft pulls, never rules.
enum class Style { BoomBap, LoFi, Strung, NeoSoul };
constexpr int kNumStyles = 4;

struct StyleInfo {
  const char* name;
  const char* odds[kLanes]; // per lane: sixteen digits, the chance of a hit in tenths
  double swing;             // where the swing dial goes when the style is chosen
  double late, loose;       // the nudge's lean (late is positive) and scatter, in steps
  double rolls;             // the chance a hat step is a roll
  double softness;          // velocity scale
  int bpm;                  // typical tempo, shown only
};
const StyleInfo& styleInfo(Style s);

struct Pattern {
  std::array<std::array<float, kSteps>, kLanes> vel{};          // 0 is a rest
  std::array<std::array<float, kSteps>, kLanes> nudge{};        // -0.45 to 0.45 of a step
  std::array<std::array<std::uint8_t, kSteps>, kLanes> roll{};  // hits in the step, 1 to 4
  std::array<std::uint8_t, kLanes> sound{};                     // bits: which kept pads; 0 is built-in
  double swing = 0.5;

  Pattern() { for (auto& lane : roll) lane.fill(1); }
  bool on(int lane, int step) const { return vel[(size_t) lane][(size_t) step] > 0.0f; }
  int hits() const;
  bool operator==(const Pattern& o) const {
    return vel == o.vel && nudge == o.nudge && roll == o.roll && sound == o.sound && swing == o.swing;
  }
  bool operator!=(const Pattern& o) const { return !(*this == o); }
};

// What the litter calls to breed patterns. `pull` (0 to 1) says how hard the style and the swing
// dial pull. `pads` is the left panel's keep row: each pad's zone, or -1 if empty.
struct PatternOps {
  Style style = Style::BoomBap;
  double swingDial = 0.58, pull = 0.7;
  std::array<bool, kLanes> locks{};
  std::array<int, kKitPads> pads{ -1, -1, -1, -1, -1, -1, -1, -1 };

  Pattern fresh(breed::Rng& rng) const;
  Pattern mutate(const Pattern& from, double spread, breed::Rng& rng) const;
  Pattern cross(const Pattern& a, const Pattern& b, breed::Rng& rng) const; // each lane whole from one parent

  double odds(int lane, int step) const;
  float velocityFor(int lane, int step, breed::Rng& rng) const;
  // The breeder's choice of sound for a lane: one of the pads labelled for it (any kept pad, for
  // perc, if none is), sometimes two layered. 0 if there is nothing to choose from.
  std::uint8_t drawSound(int lane, breed::Rng& rng) const;
  // The pads a lane might use, best first: those labelled for it, then the rest.
  int choices(int lane, int* out) const;
  bool plays(std::uint8_t mask) const; // whether any of the mask's pads is kept
};

struct Hit { double beat; int lane; float velocity; };
// Every hit of the bar, in beats from 0 to 4, with swing, nudges and rolls applied.
int hitsOf(const Pattern& p, Hit* out, int max);
constexpr int kMaxHits = kLanes * kSteps * 4;

struct Slot { bool full = false; Pattern loop; };

class Loops {
public:
  static constexpr int kLitter = 4, kSlots = 8;

  explicit Loops(std::uint32_t seed = 1);

  const breed::Litter<Pattern>& litter() const { return litter_; }
  const PatternOps& ops() const { return ops_; }
  const Slot& slot(int s) const { return slots_[(size_t) s]; }
  int playingSlot() const { return playingSlot_; } // -1 when the heard tile plays
  const Pattern& playing() const;                  // what the loop plays and the inspector shows

  void select(int i) { litter_.select(i); playingSlot_ = -1; }
  void breedFrom(int i) { litter_.breed(i, ops_); playingSlot_ = -1; }
  void cross(int i, int j) { litter_.cross(i, j, ops_); playingSlot_ = -1; }
  void fresh() { litter_.refresh(ops_); playingSlot_ = -1; }
  bool back() { playingSlot_ = -1; return litter_.back(); }

  void setStyle(Style s);
  void setSwingDial(double s);
  void setPull(double p);
  void setLock(int lane, bool on) { if (lane >= 0 && lane < kLanes) ops_.locks[(size_t) lane] = on; }
  void setPads(const std::array<int, kKitPads>& zones) { ops_.pads = zones; }
  // Kit pads a and b swapped places: every loop follows its sounds to their new pads.
  void swapPadBits(int a, int b);

  // By hand, on what is playing. A step goes rest, hit, ghost, rest.
  void cycleStep(int lane, int step);
  void setSwing(double s);
  void cycleSound(int lane); // through the lane's choices one pad at a time, then built-in

  // Unlike the drums, keeping a loop leaves the litter as it is, for variations on it.
  void keep(int tile, int s);
  void swapSlots(int a, int b);
  void clearSlot(int s);
  void playSlot(int s);
  void breedFromSlot(int s); // the slot's loop becomes the parent of a new litter

  void save(std::ostream&) const;
  bool load(std::istream&);

private:
  template <class F> void change(F f);

  PatternOps ops_;
  breed::Litter<Pattern> litter_;
  std::array<Slot, kSlots> slots_{};
  int playingSlot_ = -1;
};

} // namespace bb2
